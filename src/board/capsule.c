// Board capsule inventory, selection, throwing, collision, and visual effects.
#include "dolphin/math.h"

#include "game/board/masu.h"

#include "game/board/audio.h"

#include "game/board/branch.h"

#include "game/board/capsule.h"

#include "game/board/main.h"

#include "game/board/object.h"

#include "game/board/player.h"

#include "game/board/tutorial.h"

#include "game/board/camera.h"

#include "game/board/effect.h"

#include "game/board/window.h"

#include "game/gamework.h"

#include "game/charman.h"

#include "game/hsfload.h"

#include "game/hu3d.h"

#include "game/main.h"

#include "game/memory.h"

#include "game/object.h"

#include "game/process.h"

#include "game/pad.h"

#include "game/printfunc.h"

#include "game/sprite.h"

#include "game/gamemes.h"

#include "humath.h"

#include "messdir_enum.h"

#include "float.h"

#include "string.h"

#define M_PI 3.141592653589793

#define CAPSULE_OBJ_COLOR_MAX 128

// Hardware tick budget for each throw-path collision search update.
#define CAPSULE_THROW_SEARCH_TICK_BUDGET 100000

#define CAPSULE_VALUE_TYPE_BYTE_MASK 0xFF

#define CAPSULE_MASU_SELECT_BLOCKED_FLAG (1 << 15)

enum {
    CAPSULE_BOMHEI = 24,
    CAPSULE_HONE = 30,
    CAPSULE_KETTOU = 41,
    CAPSULE_KOOPA = 43,
    CAPSULE_DICE = 47,
    CAPSULE_YAMERU = 48,
    CAPSULE_MAX = 53,
};

enum {
    CAPSULE_DATA_KINOKO = 0,
    CAPSULE_DATA_S_KINOKO = 1,
    CAPSULE_DATA_P_KINOKO = 2,
    CAPSULE_DATA_M_KINOKO = 3,
    CAPSULE_DATA_KILLER = 4,
    CAPSULE_DATA_DOKAN = 5,
    CAPSULE_DATA_HANACHAN = 6,
    CAPSULE_DATA_N_KINOKO = 7,
    CAPSULE_DATA_TOGEZO = 8,
    CAPSULE_DATA_KURIBO = 9,
    CAPSULE_DATA_PAKKUN = 10,
    CAPSULE_DATA_JANGO = 11,
    CAPSULE_DATA_KOKAMEKKU = 12,
    CAPSULE_DATA_KAMEKKU = 13,
    CAPSULE_DATA_THROWMAN = 14,
    CAPSULE_DATA_BOBLE = 15,
    CAPSULE_DATA_BIRIQ = 16,
    CAPSULE_DATA_TUMUJIKUN = 17,
    CAPSULE_DATA_DOSSUN = 18,
    CAPSULE_DATA_BOMUHEI = 19,
    CAPSULE_DATA_PATAPATA = 20,
    CAPSULE_DATA_HONE = 21,
    CAPSULE_DATA_LIGHT = 22,
    CAPSULE_DATA_BORDER_0 = 24,
    CAPSULE_DATA_BORDER_1 = 25,
    CAPSULE_DATA_BORDER_2 = 26,
    CAPSULE_DATA_BORDER_3 = 27,
    CAPSULE_DATA_OBJ_TYPE_0 = 28,
    CAPSULE_DATA_OBJ_TYPE_1 = 29,
    CAPSULE_DATA_OBJ_TYPE_2 = 30,
    CAPSULE_DATA_OBJ_TYPE_3 = 31,
    CAPSULE_DATA_OBJ_TYPE_4 = 32,
    CAPSULE_DATA_ENTRY_33 = 33,
    CAPSULE_DATA_YAMERU = 34,
    CAPSULE_DATA_OBJ_ANIM = 39,
    CAPSULE_DATA_REMOVE_ANIM = 40,
    CAPSULE_DATA_HILITE_0 = 49,
    CAPSULE_DATA_HILITE_1 = 50,
    CAPSULE_DATA_HILITE_2 = 51,
    CAPSULE_DATA_INIT_MODEL_0 = 61,
    CAPSULE_DATA_INIT_MODEL_1 = 67,
    CAPSULE_DATA_LIST_0 = 74,
    CAPSULE_DATA_LIST_1 = 75,
    CAPSULE_DATA_LIST_2 = 76,
    CAPSULE_DATA_LIST_3 = 77,
    CAPSULE_DATA_LIST_4 = 78,
    CAPSULE_DATA_LIST_5 = 79,
    CAPSULE_DATA_LIST_6 = 80,
    CAPSULE_DATA_LIST_7 = 81,
    CAPSULE_DATA_LIST_8 = 82,
    CAPSULE_DATA_LIST_9 = 83,
    CAPSULE_DATA_LIST_10 = 84,
    BOARD_DATA_DICE = 23,
};

enum {
    CAPSULE_MESSAGE_KINOKO = 0,
    CAPSULE_MESSAGE_S_KINOKO = 1,
    CAPSULE_MESSAGE_P_KINOKO = 2,
    CAPSULE_MESSAGE_M_KINOKO = 3,
    CAPSULE_MESSAGE_KILLER = 4,
    CAPSULE_MESSAGE_DOKAN = 5,
    CAPSULE_MESSAGE_HANACHAN = 6,
    CAPSULE_MESSAGE_N_KINOKO = 7,
    CAPSULE_MESSAGE_TOGEZO = 8,
    CAPSULE_MESSAGE_KURIBO = 9,
    CAPSULE_MESSAGE_PAKKUN = 10,
    CAPSULE_MESSAGE_JANGO = 11,
    CAPSULE_MESSAGE_PATAPATA = 12,
    CAPSULE_MESSAGE_KOKAMEKKU = 13,
    CAPSULE_MESSAGE_KAMEKKU = 14,
    CAPSULE_MESSAGE_THROWMAN = 15,
    CAPSULE_MESSAGE_BOBLE = 16,
    CAPSULE_MESSAGE_BIRIQ = 17,
    CAPSULE_MESSAGE_TUMUJIKUN = 18,
    CAPSULE_MESSAGE_DOSSUN = 19,
    CAPSULE_MESSAGE_BOMUHEI = 20,
    CAPSULE_MESSAGE_HONE = 21,
    CAPSULE_MESSAGE_LIGHT = 22,
    CAPSULE_MESSAGE_TARU = 23,
    CAPSULE_MESSAGE_KILLER_MOVE = 24,
    CAPSULE_MESSAGE_KETTOU = 25,
    CAPSULE_MESSAGE_MIRACLE = 26,
    CAPSULE_MESSAGE_KOOPA = 27,
    CAPSULE_MESSAGE_DONKEY = 28,
    CAPSULE_MESSAGE_VS = 29,
    CAPSULE_MESSAGE_R_TERESA = 30,
    CAPSULE_EX99_MESSAGE_DICE = 31,
    CAPSULE_EX99_MESSAGE_YAMERU = 32,
    CAPSULE_EX99_MESSAGE_DEBUG_CAM = 33,
    CAPSULE_EX99_MESSAGE_DEBUG_WARP = 34,
    CAPSULE_EX99_MESSAGE_DEBUG_SETPOS = 35,
    CAPSULE_EX99_MESSAGE_SELECT_HELP = 36,
    CAPSULE_EX99_MESSAGE_END_SPECIAL = 37,
    CAPSULE_EX99_MESSAGE_DELETE_CHOICE = 38,
    CAPSULE_EX99_MESSAGE_USE_CHOICE = 39,
    CAPSULE_EX99_MESSAGE_SELECT_CAPSULE = 52,
    CAPSULE_EX99_MESSAGE_YAMERU_CHOICE = 58,
    CAPSULE_EX98_MESSAGE_DEBUG_CAM = 31,
    CAPSULE_EX98_MESSAGE_DEBUG_WARP = 32,
    CAPSULE_EX98_MESSAGE_DEBUG_SETPOS = 33,
    CAPSULE_EX98_MESSAGE_NONE = 34,
};

enum {
    CAPSULE_VALUE_TYPE_BITS = 8,
    CAPSULE_VALUE_TYPE_MASK = CAPSULE_VALUE_TYPE_BYTE_MASK,
    CAPSULE_VALUE_PLAYER_SHIFT = CAPSULE_VALUE_TYPE_BITS,
    CAPSULE_VALUE_NONE = CAPSULE_VALUE_TYPE_MASK,
    CAPSULE_MASU_SELECT_BLOCKED = CAPSULE_MASU_SELECT_BLOCKED_FLAG,
    CAPSULE_EFF_COLOR_RANGE = 32768,
    CAPSULE_VALID_LIST_MAX = 33,
};

enum {
    MSM_SE_BOARD_25 = 1029,
    MSM_SE_BOARD_26 = 1030,
    MSM_SE_BOARD_27 = 1031,
    MSM_SE_BOARD_28 = 1032,
    MSM_SE_BOARD_30 = 1034,
    MSM_SE_BOARD_36 = 1040,
};

static GXColor capsuleCrackEffColor;

static GXColor capsuleCrackEffAmbColor;

static GXColor capsuleCrackEffMatColor;

typedef void (*CAPSULE_THROW_HOOK)(BOOL startF);

typedef struct CapsuleObjColor_s {
    s16 flag; // Nonzero while this colored capsule model slot is occupied.
    MBMODELID mdlId; // Board model ID returned to capsule animations and shop displays.
    MBMODELID unusedModelId; // Unused storage; set to MB_MODEL_NONE when the slot is released.
    u8 layer; // Render layer last assigned through the colored capsule interface.
    HuVecF pos; // Stored base position in board world units.
    HuVecF rot; // Stored model rotation in degrees.
    HuVecF scale; // Stored per-axis model scale.
} CAPSULE_OBJ_COLOR;

typedef struct CapEffMasuOkWork_s {
    s32 modelId; // Board model for an eligible-space or current-space marker.
    s32 state; // 0 idle, 1 start, 2 growing, 3 shown, 10 shrinking, 11 hidden.
    s32 masuId; // Followed board space; zero leaves the marker unassigned.
    s32 unusedWord; // Cleared at creation and otherwise unused.
    float scale; // Uniform scale of the animated current-space marker.
} CAP_EFF_MASU_OK_WORK;

typedef struct CapEffRemoveWork_s {
    int modelId; // Engine model holding the capsule-removal particles.
    int activeCount; // Particles still playing their sixteen-frame texture animation.
    void *anim; // Texture animation released after the removal effect ends.
} CAP_EFF_REMOVE_WORK;

typedef struct CapEffHiliteWork_s {
    int modelId[3]; // Engine models for the three selectable highlight textures.
    int activeCount; // Active highlight count shared by all three texture models.
    ANIMDATA *anim[3]; // Highlight textures released with the selection effects.
} CAP_EFF_HILITE_WORK;

typedef struct CapEffCrackData_s {
    BOOL flag; // TRUE while this shell triangle is visible.
    float scale; // Triangle size multiplier, reduced during the outward burst.
    float angle; // Triangle rotation in degrees around the effect's Z axis.
    float angleSpeed; // Degrees added per object update after the delay expires.
    float scaleSpeed; // Scale removed per object update during the burst.
    int delay; // Object updates remaining before this triangle starts moving.
    HuVecF prevVel; // Initial triangle center used when raising the flat shell into a dome.
    HuVecF prevPos[3]; // Initial vertex offsets from the triangle center.
    HuVecF accel; // Outward displacement added to the triangle center each update.
    HuVecF vel; // Current triangle center in effect-local world units.
    HuVecF pos[3]; // Current vertex offsets from the triangle center.
    HuVec2f uv[3]; // Texture coordinates for the triangle's three vertices.
    GXColor color; // Triangle tint and opacity.
} CAP_EFF_CRACK_DATA;

typedef struct CapEffCrackWork_s {
    s32 modelId; // Engine model whose draw hook renders the capsule shell.
    s32 state; // 0 idle, 1 flatten, 2 expand, 3 raise, 4 burst; 5 leaves it unchanged.
    s32 time; // Object-update counter for the expanding and raising phases.
    s32 num; // Number of shell triangles, including hidden triangles outside the circle.
    s32 vtxNum; // Three render vertices per shell triangle.
    s32 segNum; // Cleared at creation and otherwise unused.
    HuVecF *vtx; // Render vertex buffer rebuilt by the draw hook.
    HuVec2f *st; // Render texture-coordinate buffer rebuilt by the draw hook.
    CAP_EFF_CRACK_DATA *data; // Motion, shape, and color of each shell triangle.
    ANIMDATA *animP; // Shell texture animation.
    GXColor color; // Additional color used while the shell rises and bursts.
    u32 dlSize; // Size of the indexed triangle display list in bytes.
    void *dl; // Indexed triangle display list.
} CAP_EFF_CRACK_WORK;

typedef struct CapEffTrailWork_s {
    HuVecF prevPos[12]; // Ring buffer of the thrown capsule's recent world positions.
} CAP_EFF_TRAIL_WORK;

typedef struct CapEffTrailPoint_s {
    HuVecF start; // More recent endpoint of this recorded flight segment.
    HuVecF end; // Older endpoint of this recorded flight segment.
    float mag; // Segment length in board world units.
    float totalMag; // Distance from the trail head to this segment's start.
} CAP_EFF_TRAIL_POINT;

#define CAP_EFF_ATTR_NONE 0

#define CAP_EFF_ATTR_COUNTER_RESET (1 << 0)

#define CAP_EFF_ATTR_COUNTER_UPDATE (1 << 1)

#define CAP_EFF_DISPATTR_NONE 0

#define CAP_EFF_DISPATTR_ZBUF_OFF (1 << 0)

#define CAP_EFF_DISPATTR_NOANIM (1 << 1)

#define CAP_EFF_DISPATTR_CAMERA_ROT (1 << 2)

#define CAP_EFF_DISPATTR_ROT3D (1 << 3)

#define CAP_EFF_DISPATTR_ALL 15

#define CAP_EFF_GLOW_Z_OFFSET (30.0f + (16.0f * FLT_EPSILON))

typedef struct CapEffect_s CAP_EFFECT;

typedef void (*CAP_EFF_HOOK)(HU3D_MODEL *modelP, CAP_EFFECT *effP, Mtx *matrix);

typedef struct CapEffData_s {
    s16 phase; // Highlight phase: zero fades in, one fades out; removal particles clear it.
    s16 fadeFrame; // Number of object updates elapsed in the current highlight phase.
    s16 easingMode; // Highlight easing: zero linear, one sine, two complementary cosine.
    s16 cameraBit; // Unused particle storage.
    HuVecF vel; // Removal velocity, or highlight start ratio, end ratio, and full size.
    float fadeInDuration; // Highlight fade-in duration in object updates.
    float fadeOutDuration; // Highlight fade-out duration in object updates.
    float speed; // Removal rotation increment, or the highlight's maximum alpha value.
    float unusedFloat; // Unused storage in a capsule effect particle.
    float gravity; // Unused particle storage.
    float rotSpeed; // Unused particle storage.
    float animTime; // Fractional frame of the removal texture animation.
    float animSpeed; // Removal texture frames advanced per object update.
    float scale; // Quad size in world units; zero makes the slot inactive.
    HuVecF rot; // Degree angles for 3D highlights; ordinary quads use rot.z as radians.
    HuVecF pos; // Quad center in effect-local world units.
    GXColor color; // Per-quad tint and opacity.
    int frameNo; // Cell in the sixteen-frame texture atlas; ignored for full-texture highlights.
} CAP_EFF_DATA;

struct CapEffect_s {
    s16 mode; // Initialized to zero and otherwise unused by capsule effects.
    s16 time; // Initialized to zero and otherwise unused by capsule effects.
    HuVecF vel; // Unused effect storage.
    s16 work[8]; // Unused effect storage.
    u8 blendMode; // Normal, additive, or inverse-color quad blending.
    u8 attr; // CAP_EFF_ATTR_* flags controlling the render counter.
    u8 dispAttr; // CAP_EFF_DISPATTR_* flags controlling quad geometry, UVs, and depth tests.
    u8 unusedByte; // Cleared when the effect is created; otherwise unused.
    HU3D_MODELID modelId; // Unused model-ID storage.
    s16 num; // Number of particle slots and indexed quads in this effect.
    u32 count; // Counter advanced by normal render passes unless its update flag is set.
    u32 prevCounter; // Last GlobalCounter value drawn, preventing repeated normal draws.
    u32 prevCount; // Nonzero counter limit applied after drawing; zero leaves the count
                   // unrestricted.
    u32 dlSize; // Size of the indexed quad display list in bytes.
    ANIMDATA *anim; // Shared texture animation for the quad pool.
    CAP_EFF_DATA *data; // Particle state and tint, one entry per quad.
    HuVecF *vertex; // Four render vertices per particle, rebuilt by the draw hook.
    HuVec2f *st; // Four texture coordinates per particle, rebuilt by the draw hook.
    void *dl; // Indexed quad display list.
    CAP_EFF_HOOK hook; // Optional callback run before quad geometry is rebuilt.
    HU3D_MODEL *hookMdlP; // Optional preceding effect model drawn through the same matrix.
}

;

typedef struct CapEffUseWork_s {
    int playerNo; // Board player whose capsule effect is running.
    int capsuleNo; // Capsule type used by the effect.
} CAP_EFF_USE_WORK;

typedef struct CapSelectMasuWork_s {
    int playerNo; // Board player selecting a capsule destination.
    int unusedResult; // Initialized to -1 and not read by either selection path.
    int capsuleNo; // Capsule type whose placement rules and appearance are used.
    int capObjId; // Capsule preview model; MB_MODEL_NONE when released or restored.
    int cameraObjId; // Hidden board model used as the selection camera's focus.
    int helpWinId; // Control help window, or MB_MODEL_NONE when absent.
    int messageWinId; // Destination prompt window, or MB_MODEL_NONE when absent.
} CAP_SELECT_MASU_WORK;

typedef struct CapEffThrowWork_s {
    int modelId; // Engine hook model that advances the path search during rendering.
    int playerNo; // Player excluded from collision checks; -2 disables player collision.
    int maxTime; // Flight duration in frames, with a minimum of 24.
    BOOL endF; // TRUE when a candidate or fallback path is ready.
    int minNo; // Initial candidate index, set to zero at creation.
    int no; // Current entry in the 48-candidate path table.
    int initF; // Zero starts a candidate; otherwise its next collision sample index.
    HuVecF startPos; // World-space launch position.
    HuVecF endPos; // World-space destination position.
    HuVecF pos; // Previous point used by the segment collision tests.
    float yOfs; // Extra arc height in world units.
    u32 tick; // Tick at the start of the current render-hook search slice.
    u32 delay; // Board-specific tick budget checked after each collision-free sample.
    float x[3]; // Launch, candidate bend, and destination X coordinates.
    float y[3]; // Launch, candidate bend, and destination Y coordinates.
    float z[3]; // Launch, candidate bend, and destination Z coordinates.
} CAP_EFF_THROW_WORK;

typedef struct CapPlayerThrowFrame_s {
    float speed; // Fraction of the launch-to-destination vector used for the bend.
    float yOfs; // Extra bend height in world units.
    float radius; // Fixed sideways bend offset in world units; zero uses dir instead.
    int dir; // Direction multiplier for a sideways offset based on throw distance.
} CAP_PLAYER_THROW_FRAME;

typedef struct CapGuideWork_s {
    int objId; // Board model showing one route from the selected space.
    int state; // 0 grows the guide; 1 keeps its pulse phase advancing.
    int rotY; // Pulse phase in degrees, advanced by the object update.
    float scale; // Uniform guide scale; shrinks below zero before being hidden.
} CAP_GUIDE_WORK;

typedef struct CapsuleObjData_s {
    int objId; // Cancel-item board model, or MB_MODEL_NONE in an unused slot.
    void *anim; // Sprite animation shared by both animated model textures.
    HU3D_ANIMID textureAnimId; // Texture animation attached to S3TCys77120.
    HU3D_ANIMID linkedTextureAnimId; // Linked texture animation attached to S3TCys77121.
} CAPSULE_OBJ_DATA;

typedef struct CapPlayerThrowWork_s {
    int playerNo; // Board player performing the throw.
    int masuId; // Destination board space.
    int capsuleNo; // Capsule type being placed.
    int capObjId0; // Capsule model moved into the hand and hidden at release.
    int capObjId1; // Hidden capsule model used to focus the camera during flight.
    int objColorId; // Colored capsule model displayed during flight.
    int jumpMotId; // Player motion used for the throwing pose.
    int time; // Elapsed flight frames.
    int maxTime; // Duration selected by the throw-path search, in frames.
    float yOfs; // Arc height above the endpoints, capped at 700 world units.
    HuVecF pos; // Hand position at release in world coordinates.
    HuVecF masuPos; // Destination position, raised 20 world units above its space.
    HuVecF unused; // Cleared with the work record and otherwise unused.
} CAP_PLAYER_THROW_WORK;

typedef struct CapAutoThrowWork_s {
    int playerNo; // Player receiving vibration and ownership of the placed capsule.
    int capsuleNo; // Capsule type being placed.
    int masuId; // Destination space; nonpositive skips space transforms and ending window.
    int maxTime; // Duration of the supplied flight, in frames.
    float startT; // Starting fraction of the flight; at least one skips flight playback.
    HuVecF startPos; // Launch point in world coordinates.
    HuVecF endPos; // Middle control point in world coordinates.
    HuVecF masuPos; // Final control point in world coordinates.
} CAP_AUTO_THROW_WORK;

typedef struct CapUseDeleteWork_s {
    int playerNo; // Board player answering the discard prompt.
    int unusedModelId; // Initialized to MB_MODEL_NONE and otherwise unused.
    int capsuleNo; // Capsule type presented for discarding.
    int capObjId; // Capsule preview model; MB_MODEL_NONE once released or restored.
    int auxModelId; // Optional model released by cleanup; this path leaves it absent.
    int primaryWinId; // Optional window released by cleanup; this path leaves it absent.
    int secondaryWinId; // Optional window released by cleanup; this path leaves it absent.
    int unusedTail; // Zeroed with the work record and otherwise unused.
} CAP_USE_DELETE_WORK;

typedef struct CapUseWork_s {
    int playerNo; // Board player choosing whether to use the capsule.
    int capsuleNo; // Capsule type shown in the use prompt.
} CAP_USE_WORK;

typedef struct CapsuleList_s {
    s8 id; // Capsule type, or -1 at the end of a distribution list.
    s8 cost[3]; // Price alternatives used by purchase and sale rank rules.
    s8 weight[12]; // Four rank weights each for early rewards, late rewards, and shops.
} CAPSULE_LIST;

typedef struct CapsuleListFile_s {
    s32 boardNo; // Board whose capsule list is stored here; negative ends the table.
    s32 dataNo; // Data archive entry containing this board's capsule list.
} CAPSULE_LIST_FILE;

typedef struct CapsuleData_s {
    u32 file; // Item model asset used by capsule selection and use animations.
    u32 objFile; // Colored capsule model asset used during flight and other capsule effects.
    u32 descMes; // Message describing this capsule in selection and shop windows.
    u32 useMes; // Capsule name message inserted into use and discard prompts.
    int masuPat; // Space-display pattern associated with this capsule type.
    int color; // Capsule color category used by models and trail tints.
    int cost; // Default coin cost when the board list has no positive price override.
    s8 code; // Letter category used by rank- and turn-weighted capsule generation.
    u16 useMode; // Capsule use mode; mode two is tested as trap placement.
    char *debugName; // Name displayed by the capsule debug screens.
    u8 listFlag; // Value returned by the capsule-list exclusion query.
} CAPSULE_DATA;

typedef struct CapsuleTurnData_s {
    s8 code; // Weighted capsule letter category, from A through E.
    int chance; // Cumulative weight threshold for this category's random selection.
} CAPSULE_TURN_DATA;

typedef struct CapsuleComChoice_s {
    int index; // Position in the caller's capsule inventory list.
    int capsuleNo; // Capsule type at that position.
    int chance; // Computer discard weight used to rank this choice.
} CAPSULE_COM_CHOICE;

typedef struct CapsuleComChoiceBack_s {
    int index; // Position in the caller's capsule inventory list.
    int capsuleNo; // Capsule type at that position.
    int chance; // Adjusted computer use weight.
    BOOL back; // TRUE when this choice favors a backward destination.
} CAPSULE_COM_CHOICE_BACK;

typedef struct CapsuleComChanceRank_s {
    s16 chance; // Base use weight for one character.
    s8 code; // Character-specific modifier code used by capsule decision rules.
    s8 unusedByte; // Unused byte in the computer-choice rank entry.
} CAPSULE_COM_CHANCE_RANK;

typedef struct CapsuleComChance_s {
    s16 capsuleNo; // Capsule type; -1 ends the computer-weight table.
    CAPSULE_COM_CHANCE_RANK rank[11]; // Base weights and modifier codes by character.
} CAPSULE_COM_CHANCE;

static CAPSULE_THROW_HOOK capsuleThrowHook;

static s16 *capsuleBorderObjId;

static OMOBJ *capsuleThrowGlowOMObj;

static OMOBJ *capsuleThrowRingOMObj;

static OMOBJ *capsuleThrowRayOMObj;

static OMOBJ *capsuleThrowMasuHitOMObj;

static OMOBJ *capsuleThrowMasuCoinOMObj;

static BOOL capsuleUseRemoveOnF;

static BOOL capsuleMasuSelectEndF;

static int capsuleComChoice;

static OMOBJ *capEffTrailOMObj;

static OMOBJ *capEffCrackOMObj;

static OMOBJ *capEffHiliteOMObj;

static OMOBJ *capEffRemoveAddOMObj;

static OMOBJ *capEffRemoveOMObj;

static BOOL capsuleSelectComBack;

static OMOBJ *capEffMasuOkOMObj;

static OMOBJ *capsuleGuideOMObj;

static BOOL capsuleComSearchF;

static HUPROCESS *capsulePlayerThrowProc;

static int capsuleMasuSelectResult = MB_MODEL_NONE;

static int capsuleObjId = MB_MODEL_NONE;

static int capsuleColObjId = MB_MODEL_NONE;

static int capsuleColMdlId = MB_MODEL_NONE;

static int capEffThrowMdlId = MB_MODEL_NONE;

static BOOL capsuleMasuSelectComF[4] = { FALSE, FALSE, FALSE, FALSE };

static CAPSULE_DATA capsuleData[] = {
    { DATANUM(DATA_capsule, CAPSULE_DATA_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KINOKO), 0, 0, 5, 'A', 0, "KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_S_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_S_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_S_KINOKO), 0, 0, 10, 'B', 0, "S KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_P_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_P_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_P_KINOKO), 0, 0, 10, 'A', 0, "P KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_M_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_M_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_M_KINOKO), 0, 0, 20, 'A', 0, "M KINOKO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KILLER), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KILLER), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KILLER), 0, 0, 20, 'B', 0, "KILLER", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_DOKAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DOKAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DOKAN), 0, 0, 15, 'C', 0, "DOKAN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_HANACHAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_HANACHAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_HANACHAN), 0, 0, 30, 'E', 0, "HANACHAN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_N_KINOKO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_0), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_N_KINOKO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_N_KINOKO), 0, 0, 30, 'E', 0, "N KINOKO", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NULL", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NASI", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_TOGEZO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TOGEZO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TOGEZO), 1, 1, 10, 'A', 1, "TOGEZO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KURIBO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KURIBO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KURIBO), 1, 1, 15, 'A', 1, "KURIBO", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_PAKKUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_PAKKUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_PAKKUN), 1, 1, 20, 'B', 1, "PAKKUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_JANGO), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_JANGO), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_JANGO), 1, 1, 5, 'D', 1, "JANGO", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "HANUKE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KOKAMEKKU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KOKAMEKKU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KOKAMEKKU), 1, 1, 10, 'C', 1, "KOKAMEKKU", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_KAMEKKU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KAMEKKU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KAMEKKU), 1, 1, 5, 'B', 1, "KAMEKKU", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_THROWMAN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_1), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_THROWMAN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_THROWMAN), 1, 1, 10, 'A', 1, "THROWMAN", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "SUKA", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "KARA", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BOBLE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BOBLE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BOBLE), 2, 2, 10, 'B', 2, "BOBLE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BIRIQ), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BIRIQ), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BIRIQ), 2, 2, 15, 'C', 2, "BIRIQ", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_TUMUJIKUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TUMUJIKUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TUMUJIKUN), 2, 2, 15, 'B', 2, "TUMUJIKUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_DOSSUN), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DOSSUN), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DOSSUN), 2, 2, 15, 'C', 2, "DOSSUN", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_BOMUHEI), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_BOMUHEI), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_BOMUHEI), 2, 2, 10, 'C', 2, "BOMUHEI", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_PATAPATA), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_2), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_PATAPATA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_PATAPATA), 2, 2, 10, 'E', 2, "PATAPATA", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "NETA", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "GA  ", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "TUKI", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "TA  ", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_HONE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_HONE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_HONE), 3, 3, 20, 'D', 3, "HONE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_LIGHT), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_LIGHT), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_LIGHT), 3, 3, 10, 'D', 3, "LIGHT", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_TARU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_TARU), 3, 3, 15, 'D', 3, "TARU", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KILLER_MOVE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KILLER_MOVE), 10, 3, 1, 'C', 3, "KILLER MOVE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KETTOU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KETTOU), 10, 3, 1, 'C', 3, "KETTOU", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_4), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_MIRACLE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_MIRACLE), 10, 3, 1, 'C', 3, "MIRACLE", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_KOOPA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_KOOPA), 9, 3, 1, 'E', 3, "KOOPA", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_DONKEY), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_DONKEY), 10, 3, 1, 'E', 3, "DONKEY", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_VS), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_VS), 10, 3, 1, 'Z', 3, "VS", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_MESSAGE_R_TERESA), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_MESSAGE_R_TERESA), 10, 3, 1, 'Z', 3, "R_TERESA", 0 },
    { DATANUM(DATA_board, BOARD_DATA_DICE), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DICE), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "DICE", 1 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_YAMERU), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_YAMERU), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 0, "YAMERU", 1 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_CAM), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_CAM), 10, 0, 1, 'Z', 0, "DEBUG CAM TEST", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_WARP), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_WARP), 10, 0, 1, 'Z', 0, "DEBUG WARP TEST", 0 },
    { DATANUM(DATA_capsule, CAPSULE_DATA_ENTRY_33), DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_TYPE_3), MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DEBUG_SETPOS), MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_DEBUG_SETPOS), 10, 0, 1, 'Z', 0, "DEBUG SETPOS TEST", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
    { 0, 0, 0, MESSNUM(MESS_CAPSULE_EX98, CAPSULE_EX98_MESSAGE_NONE), 10, 0, 0, 'Z', 1, "0000", 0 },
};

static CAPSULE_COM_CHANCE capsuleChanceTbl[] = {
    { 0, { { 50, ' ' }, { 50, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 1, { { 50, '*' }, { 50, '*' }, { 10, '*' }, { 30, '*' }, { 30, ' ' }, { 30, '*' }, { 10, ' ' }, { 30, '*' }, { 30, ' ' }, { 30, ' ' }, { 30, '*' } } },
    { 2, { { 5, 'b' }, { 5, 'b' }, { 10, 'b' }, { 10, 'b' }, { 10, ' ' }, { 5, 'a' }, { 10, ' ' }, { 5, 'a' }, { 30, ' ' }, { 10, ' ' }, { 5, 'a' } } },
    { 3, { { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 0, '*' }, { 30, ' ' }, { 0, '*' }, { 0, '*' }, { 30, ' ' }, { 0, '*' } } },
    { 4, { { 10, ' ' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 10, '*' }, { 50, ' ' }, { 50, '*' }, { 30, '*' }, { 50, ' ' }, { 30, '*' } } },
    { 5, { { 30, ' ' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, '*' }, { 50, ' ' }, { 10, ' ' }, { 10, '*' } } },
    { 6, { { 30, '*' }, { 50, '*' }, { 30, '*' }, { 50, '*' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 50, '*' }, { 10, '*' }, { 10, '*' }, { 30, '*' } } },
    { 10, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' } } },
    { 11, { { 10, ' ' }, { 30, '*' }, { 30, '*' }, { 30, '*' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, '*' }, { 50, ' ' }, { 50, ' ' }, { 50, '*' } } },
    { 12, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' } } },
    { 13, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 15, { { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' } } },
    { 16, { { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 17, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' } } },
    { 20, { { 50, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 21, { { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 50, ' ' } } },
    { 22, { { 50, ' ' }, { 10, '*' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 30, '*' }, { 10, ' ' }, { 10, '*' }, { 50, ' ' }, { 30, ' ' }, { 10, '*' } } },
    { 23, { { 30, ' ' }, { 50, '*' }, { 10, '*' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 10, '*' }, { 30, ' ' }, { 30, ' ' }, { 10, '*' } } },
    { 24, { { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 30, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 30, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 25, { { 30, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 10, ' ' }, { 50, ' ' }, { 10, ' ' }, { 30, ' ' }, { 10, ' ' }, { 10, ' ' } } },
    { 30, { { 30, '*' }, { 50, ' ' }, { 30, '*' }, { 30, ' ' }, { 30, '*' }, { 50, '*' }, { 10, ' ' }, { 50, ' ' }, { 30, ' ' }, { 10, ' ' }, { 50, ' ' } } },
    { 31, { { 10, '*' }, { 50, ' ' }, { 30, '*' }, { 30, ' ' }, { 10, '*' }, { 30, '*' }, { 10, ' ' }, { 50, ' ' }, { 0, ' ' }, { 10, ' ' }, { 50, ' ' } } },
    { -1, { { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' }, { 0, ' ' } } },
};

static GXColor capsuleMasuSelectColorTbl[8] = {
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
};

static HuVecF capsulePlayerThrowRot[3] = {
    { -90.0f, 0.0f, 20.0f },
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 60.0f },
};

static GXColor capsulePlayerThrowColorTbl[4] = {
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
};

static int capsulePlayerThrowDelayTbl[11] = {
    CAPSULE_THROW_SEARCH_TICK_BUDGET, CAPSULE_THROW_SEARCH_TICK_BUDGET,
    CAPSULE_THROW_SEARCH_TICK_BUDGET, CAPSULE_THROW_SEARCH_TICK_BUDGET,
    CAPSULE_THROW_SEARCH_TICK_BUDGET, CAPSULE_THROW_SEARCH_TICK_BUDGET,
    CAPSULE_THROW_SEARCH_TICK_BUDGET, CAPSULE_THROW_SEARCH_TICK_BUDGET,
    CAPSULE_THROW_SEARCH_TICK_BUDGET, CAPSULE_THROW_SEARCH_TICK_BUDGET,
    CAPSULE_THROW_SEARCH_TICK_BUDGET,
};

static CAP_PLAYER_THROW_FRAME capsulePlayerThrowFrameTbl[48] = {
    { 0.5f, 0, 0, 0 }, { 0.2f, 0, 0, 0 }, { 0.8f, 0, 0, 0 },
    { 0.5f, 500, 0, 0 }, { 0.2f, 500, 0, 0 }, { 0.8f, 500, 0, 0 }, { 1.0f, 500, 0, 0 },
    { 1.2f, 500, 0, 0 }, { 0.2f, 0, 300, 0 }, { 0.2f, 0, -300, 0 }, { 0, 0, 300, 0 },
    { 0, 0, -300, 0 }, { 0.8f, 0, 300, 0 }, { 0.8f, 0, -300, 0 }, { 1.0f, 0, 300, 0 },
    { 1.0f, 0, -300, 0 }, { 0.2f, 0, 800, 0 }, { 0.2f, 0, -800, 0 }, { 0, 0, 800, 0 },
    { 0, 0, -800, 0 }, { 0.8f, 0, 800, 0 }, { 0.8f, 0, -800, 0 }, { 1.0f, 0, 800, 0 },
    { 1.0f, 0, -800, 0 }, { 0.2f, 500, 300, 0 }, { 0.2f, 500, -300, 0 }, { 0, 500, 300, 0 },
    { 0, 500, -300, 0 }, { 0.8f, 500, 300, 0 }, { 0.8f, 500, -300, 0 }, { 1.0f, 500, 300, 0 },
    { 1.0f, 500, -300, 0 }, { 0.2f, 500, 800, 0 }, { 0.2f, 500, -800, 0 }, { 0, 500, 800, 0 },
    { 0, 500, -800, 0 }, { 0.8f, 500, 800, 0 }, { 0.8f, 500, -800, 0 }, { 1.0f, 500, 800, 0 },
    { 1.0f, 500, -800, 0 }, { 0.5f, 500, 0, 1 }, { 0.5f, 500, 0, -1 }, { 0.7f, 500, 0, 1 },
    { 0.7f, 500, 0, -1 }, { 0.3f, 500, 0, 1 }, { 0.3f, 500, 0, -1 }, { 0.7f, 500, 0, 1 },
    { 0.7f, 500, 0, -1 },
};

static HuVecF capsuleAutoThrowRot[2] = {
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 60.0f },
};

static GXColor capsuleAutoThrowColorTbl[4] = {
    { 192, 255, 192, 255 },
    { 255, 255, 192, 255 },
    { 255, 192, 192, 255 },
    { 192, 255, 255, 255 },
};

static HuVecF capsuleMasuSelectRotTbl[2] = {
    { -90.0f, 0.0f, 0.0f },
    { 0.0f, 10.0f, 50.0f },
};

static CAPSULE_LIST_FILE capsuleListFileTbl[] = {
    { 0, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_0) },
    { 1, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_1) },
    { 2, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_2) },
    { 3, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_3) },
    { 4, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_4) },
    { 5, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_5) },
    { 6, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_6) },
    { 7, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_7) },
    { 8, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_8) },
    { 9, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_9) },
    { 10, DATANUM(DATA_capsule, CAPSULE_DATA_LIST_10) },
    { -1, -1 },
};

typedef struct CapsuleListDefine_s {
    u32 capsuleNo; // Capsule type named by the debug list export; -1 ends the table.
    char *name; // Constant name printed when exporting a debug-edited capsule list.
} CAPSULE_LIST_DEFINE;

static CAPSULE_LIST_DEFINE capsuleListDefineTbl[24] = {
    { 0, "CAPSULE_KINOKO" },
    { 1, "CAPSULE_SKINOKO" },
    { 2, "CAPSULE_PKINOKO" },
    { 3, "CAPSULE_MKINOKO" },
    { 4, "CAPSULE_KILLER" },
    { 5, "CAPSULE_DOKAN" },
    { 6, "CAPSULE_HANACHAN" },
    { 10, "CAPSULE_TOGEZO" },
    { 11, "CAPSULE_KURIBO" },
    { 12, "CAPSULE_PAKKUN" },
    { 13, "CAPSULE_JANGO" },
    { 25, "CAPSULE_PATAPATA" },
    { 15, "CAPSULE_KOKAMEKKU" },
    { 16, "CAPSULE_KAMEKKU" },
    { 17, "CAPSULE_THROWMAN" },
    { 20, "CAPSULE_BOBLE" },
    { 21, "CAPSULE_BIRIQ" },
    { 22, "CAPSULE_TUMUJIKUN" },
    { 23, "CAPSULE_DOSSUN" },
    { 24, "CAPSULE_BOMHEI" },
    { 30, "CAPSULE_HONE" },
    { 31, "CAPSULE_LIGHT" },
    { 32, "CAPSULE_TARU" },
    { -1, "CAPSULE_NULL" },
};

static int capsuleListColW[16] = {
    108, 32, 32, 48,
    32, 32, 32, 32,
    32, 32, 32, 48,
    32, 32, 32, 48,
};

extern int mbev_CapEffCoinNumGet(OMOBJ *obj);

extern int mbev_CapEffGlowDispGet(OMOBJ *obj);

extern void mbev_CapEffGlowKill(OMOBJ *obj);

extern void mbev_CapEffRingKill(OMOBJ *obj);

extern void mbev_CapEffRayKill(OMOBJ *obj);

extern void mbev_CapEffMasuHitKill(OMOBJ *obj);

extern void mbev_CapEffCoinKill(OMOBJ *obj);

extern OMOBJ *mbev_CapEffRayCreate(float scale, float speed);

extern OMOBJ *mbev_CapEffMasuHitCreate(void);

extern OMOBJ *mbev_CapEffGlowCreate(void);

extern OMOBJ *mbev_CapEffRingCreate(void);

extern OMOBJ *mbev_CapEffCoinCreate(void);

extern void mbev_CapEffRayTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);

extern void mbev_CapEffMasuHitTransformSet(OMOBJ *obj, HuVecF *pos, HuVecF *rot, Mtx *mtx);

extern int mbev_CapEffRayAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB, float scale,
                             int time);

extern void mbev_CapEffRayAlphaSet(OMOBJ *obj, float alpha);

extern int mbev_CapEffMasuHitAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rotA, HuVecF *rotB, float scale,
    float scaleY, int time);

extern int mbev_CapEffRingAdd(OMOBJ *obj, HuVecF *pos, HuVecF *rot, HuVecF *vel, int kind, int time,
    int mode, GXColor *color);

extern void mbev_CapEffCoinMultiAdd(OMOBJ *obj, HuVecF *pos, int num);

extern int mbev_CapEffGlowAdd(OMOBJ *obj, HuVecF *pos, HuVecF *vel, int time, float scale,
    float gravity, float unk, GXColor *color);

extern void mbev_CapEffColorSet(GXColor *color, int colorNo);

extern void mbWipeDissolveFadeInTime(int time);

extern void mbWipeDissolveFadeOut(void);

extern void mbev_CapPlayerMotShiftWait(int playerNo, int motNo, u32 attr, BOOL waitF);

extern void mbev_CapBonusCoinCall(int playerNo, int capsuleNo, int coinNum, BOOL waitF);

extern BOOL mbev_CapMasuMoveCheck(int masuId);

extern void mbWipeSpecialFadeOutCreate(int type, int time);

extern void mbWipeSpecialFadeInCreate(int type, int time);

extern BOOL mbCapSelectShrinkCheck(int playerNo);

extern int mbCapSelectMasu(int playerNo, int capsuleNo);

extern void mbPauseDisableSet(BOOL disableF);

extern void mbWipeFadeOut(void);

extern void mbSingleReturn(void);

extern int mbSingleCall(int mode, int arg);

extern int mbev_CapCall(int playerNo, int capsuleNo, BOOL moveF, BOOL stopF);

extern void mbCapPlayerThrow(int playerNo, int masuId, int capsuleNo);

s16 mbCapValueTypeGet(s16 value);

s16 mbCapMasuDispTypeGet(s16 masuId);

int mbCapObjCreate(int capsuleNo, BOOL flag);

int mbCapObjBorderCreate(int objId, int capsuleNo);

int mbCapFileGet(int capsuleNo);

int mbCapColorGet(int capsuleNo);

int mbCapUseMesGet(int capsuleNo);

int mbCapBonusCoinNumGet(int playerNo, int capsuleNo);

int mbCapComChanceGet(int capsuleNo, int playerNo, int mode);

char *mbCapDebugNameGet(int capsuleNo);

int mbCapObjColorCreate(int capsuleNo, BOOL createF);

void mbCapObjColorPosSet(int id, float x, float y, float z);

void mbCapObjColorScaleSet(int id, float x, float y, float z);

void mbCapObjColorDispSet(int id, BOOL dispF);

void mbCapObjColorMtxGet(int id, Mtx *mtx);

void mbCapObjColorMtxSet(int id, Mtx *mtx);

void mbCapSelectResultGet(int playerNo, int *objId, int *result);

void mbCapSelectResultReset(int playerNo);

void mbCapSelectResultSet(int playerNo, int objId, int result);

extern float mbSinDeg(float angle);

extern float mbCosDeg(float angle);

extern BOOL mbSaveNewF;

extern const float lbl_802C4514;

extern const float lbl_802C4524;

extern const float lbl_802C45F8;

extern const double lbl_802C4600;

extern const double lbl_802C4608;

extern const double lbl_802C4610;

extern const float lbl_802C4618;

extern const float lbl_802C461C;

extern const float lbl_802C4620;

extern const float lbl_802C4624;

extern const float lbl_802C4628;

extern const float lbl_802C462C;

extern const float lbl_802C4630;

extern const float lbl_802C4634;

extern const float lbl_802C465C;

extern const float lbl_802C4660;

extern const float lbl_802C4664;

extern const float lbl_802C4668;

extern const float lbl_802C466C;

extern const float lbl_802C4674;

extern const float lbl_802C4678;

extern const float lbl_802C467C;

extern u32 mbCapEffNum;

extern s16 *mbCapEffData;

#define CAP_EFF_RAND_NEXT() \
    do { \
        if (++mbCapEffNum >= 1024) { \
            mbCapEffNum = 0; \
        } \
    } while (0)

static void CapComKeyHook(void);

static void CapComChoiceSet(int choice);

static void CapEffUse(void);

static void CapEffUseKill(void);

static void CapPlayerThrow(void);

static void CapAutoThrow(CAP_AUTO_THROW_WORK *work);

static void CapEffThrowHook(HU3D_MODEL *modelP, Mtx *mtx);

static void CapEffThrowCreate(int playerNo, float *x, float *y, float *z, float yOfs, int masuId);

static void CapEffThrowMasuCreate(int masuId, int capsuleNo);

static int CapEffThrowMasu(int masuId, int capsuleNo, int playerNo, BOOL bonusF);

static void CapThrowCameraSet(float *x, float *y, float *z, int num);

static void CapThrowCameraCalc(float t, float *x, float *y, float *z, HuVecF *out, int num);

static void CapThrowEndWin(int unusedMasuId, int capsuleNo);

static void CapColMdlIdGet(void);

static void CapColKill(void);

static BOOL CapColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out);

static BOOL CapColExec(int playerNo, HuVecF *posA, HuVecF *posB, HuVecF *out);

static void CapGuideKill(void);

static void CapGuideCreate(void);

static void CapGuideRotYSet(int masuId, float rotY);

static void CapGuideGrowSet(void);

static void CapGuideOMExec(OMOBJ *obj);

static BOOL CapEffThrowCheck(HuVecF *pos, int *maxTime);

static void CapEffThrowKill(void);

static BOOL CapEffThrowMasuWait(BOOL bonusCoinCount);

static int CapUseDelete(int playerNo, int capsuleNo);

static int CapUseDeleteWin(CAP_USE_DELETE_WORK *work);

static void CapUseDeleteKill(CAP_USE_DELETE_WORK *work);

static void CapEffMasuOkKill(void);

static void CapEffMasuOkCreate(void);

static void CapEffMasuOkAddAll(s16 unused, s16 *masuFlag);

static void CapEffMasuOkPosSet(HuVecF *pos, int masuId);

static void CapEffMasuOkDispSet(BOOL dispF);

static void CapEffMasuOkNext(void);

static void CapEffMasuOkOMExec(OMOBJ *obj);

static void CapEffRemoveCreate(void);

static void CapEffRemoveOMExec(OMOBJ *obj);

static void CapEffRemoveKill(void);

static BOOL CapEffRemoveCheck(void);

static int CapEffRemoveAdd(HuVecF pos, HuVecF vel, float scale,
    float speed, float offset, float animSpeed, GXColor color);

static void CapEffRemoveAddAll(HuVecF *pos);

static void CapEffRemoveAddDestroy(void);

static void CapEffHiliteKill(void);

static void CapEffHiliteCreate(void);

static void CapEffHiliteOMExec(OMOBJ *obj);

static int CapEffHiliteAdd(HuVecF pos, HuVecF rot, HuVecF scale,
    int fadeIn, int fadeOut, int modelNo, int mode, GXColor color);

static void CapEffCrackCreate(void);

static void CapEffCrackAdd(HuVecF *pos, HuVecF *rot);

static void CapEffCrackOMExec(OMOBJ *obj);

static void CapEffCrackDraw(HU3D_MODEL *modelP, Mtx *mtx);

static void CapEffCrackKill(void);

static void CapEffTrailCreate(int capsuleNo);

static void CapEffTrailOMExec(OMOBJ *obj);

static void CapEffTrailKill(void);

static void CapEffTrailPosSet(HuVecF *pos);

static void CapEffTrailAdd(HuVecF *pos, int capsuleNo);

static HU3D_MODELID CapEffCreate(ANIMDATA *anim, s16 num);

static void CapEffDraw(HU3D_MODEL *modelP, Mtx *mtx);

static int CapUseSelect(CAP_USE_WORK *useWork);

static int CapUse(int playerNo, int capsuleNo);

static BOOL CapSelectMasuCheck(int masuId);

static void CapSelectMasuKill(CAP_SELECT_MASU_WORK *work);

static BOOL CapCheckComPath(int playerNo, int max, int mode);

static void CapSelectMasuLinkCheck(s16 *masuFlag, s16 masuId);

static void CapSelectMasuListGet(
    s16 *masuFlag, s16 masuId, s16 frontMax, s16 backMax);

static void CapSelectMasuAddFront(s16 *masuFlag, s16 masuId, s16 max);

static void CapSelectMasuAddBack(s16 *masuFlag, s16 masuId, s16 max);

static int CapSelectMasuWinCreate(int messageType);

static int CapSelectMasuPlayer(CAP_SELECT_MASU_WORK *work);

static int CapSelectMasuCom(CAP_SELECT_MASU_WORK *work);

static int CapSelectMasuComListGet(
    s16 *path, s16 *masuFlag, s16 masuId, s16 targetId, int depth);

static int CapSelectMasuComListGetRev(
    s16 *path, s16 *masuFlag, s16 masuId, s16 targetId, int depth);

static float CapAngleSumWrap(float targetAngle, float stickAngle);

static float CapCameraXZAngleGet(float angle);

void mbCapAutoThrowEnd(CAP_AUTO_THROW_WORK *work);

static HUPROCESS *capsuleUseEffProc[4];

static int capsuleUseEffMode[4];

static HuVecF capsuleUseEffPos[4];

static s16 capsuleNum[33][2];

static CAPSULE_LIST capsuleList[33];

static CAPSULE_OBJ_COLOR capsuleObjColorData[CAPSULE_OBJ_COLOR_MAX];

static s16 capsuleObjBorderId[6];

static CAPSULE_OBJ_DATA capsuleObjData[8];

static float capsuleTime[8];

static float capsuleBezierX[8];

static float capsuleBezierY[8];

static float capsuleBezierZ[8];

extern s8 mbPadStkXGet(s32 playerNo);

extern s8 mbPadStkYGet(s32 playerNo);

void mbCapObjKill(int objId);

void mbCapObjColorKill(int id);

void mbCapObjColorLayerSet(int id, u8 layer);

void mbCapObjColorPosSetV(int id, HuVecF *pos);

void mbCapMasuCapsuleSet(int masuId, int capsuleNo, int playerNo);

int mbCapCostGet(s16 capsuleNo);

BOOL mbCapListExcludeCheck(s16 capsuleNo);

static void CapPlayerThrowKill(void);

// Handles the selected capsule action and starts its use or throw.
// Called by the capsule-selection flow after the player chooses a capsule.
int mbCapUse(int playerNo, int capsuleNo)
{
    int capsuleObjId;
    int masuSelectionResult;
    int gameMessageId;
    BOOL isPartyMode;
    BOOL isPartyModeAfterThrow;

    capsuleNo = mbCapValueTypeGet((s16)capsuleNo);
    capsuleUseRemoveOnF = FALSE;
    if (capsuleNo == CAPSULE_DICE) {
        int selectedResult;

        while (!mbCapSelectShrinkCheck(playerNo)) {
            HuPrcVSleep();
        }
        // The dice branch uses only the spawned object; this result is discarded.
        mbCapSelectResultGet(playerNo, &capsuleObjId, &selectedResult);
        mbCapSelectResultReset(playerNo);
        if (capsuleObjId != MB_MODEL_NONE) {
            mbCapObjKill(capsuleObjId);
        }
        return TRUE;
    }
    if (capsuleNo == CAPSULE_YAMERU) {
        mbWinCreateChoice(
            MBWIN_TYPE_EVENT,
            MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_YAMERU_CHOICE),
            -1, TRUE);
        if (GwPlayer[playerNo].comF) {
            CapComChoiceSet(0);
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() == 0) {
            mbPauseDisableSet(TRUE);
            gameMessageId = GameMesCreate(GAMEMES_MES_MG_BATTLE, TRUE);
            while (GameMesStatGet((s16)gameMessageId) != GAMEMES_STAT_NONE) {
                HuPrcVSleep();
            }
            mbWipeFadeOut();
            mbSingleReturn();
        }
        return FALSE;
    }
    if (mbPlayerCapsuleFind(playerNo, capsuleNo) == -1) {
        return TRUE;
    }
    masuSelectionResult = mbCapSelectMasu(playerNo, capsuleNo);
    if (masuSelectionResult != -1) {
        if (masuSelectionResult == -2) {
            if (CapUseDelete(playerNo, capsuleNo)) {
                return TRUE;
            }
            return FALSE;
        }
        if (masuSelectionResult == -3) {
            if (CapUse(playerNo, capsuleNo)) {
                mbev_CapCall(playerNo, capsuleNo, TRUE, FALSE);
                isPartyMode = GwSystem.partyF;
                if (!isPartyMode) {
                    mbSingleCall(4, capsuleNo);
                }
            } else {
                return FALSE;
            }
        } else {
            mbCapPlayerThrow(playerNo, masuSelectionResult, capsuleNo);
            isPartyModeAfterThrow = GwSystem.partyF;
            if (!isPartyModeAfterThrow) {
                mbSingleCall(4, capsuleNo);
            }
            return TRUE;
        }
    } else {
        return FALSE;
    }
    return TRUE;
}

void MBCapsuleStub1(void)
{
}

BOOL MBCapsuleStub2(void)
{
    return FALSE;
}

// Presents the selected capsule and updates inventory unless capsuleUseRemoveOnF is set.
// Called by mbCapUse for capsules already in inventory.
static int CapUse(int playerNo, int capsuleNo)
{
    CAP_USE_WORK *useWork;
    int result;
    CAP_USE_WORK *allocatedWork;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    allocatedWork = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAP_USE_WORK), HU_MEMNUM_OVL);
    useWork = allocatedWork;
    memset(useWork, 0, sizeof(CAP_USE_WORK));
    useWork->playerNo = playerNo;
    useWork->capsuleNo = capsuleNo;
    result = CapUseSelect(useWork);
    HuMemDirectFree(useWork);
    return result;
}

// Animates the capsule into view, asks whether to use it, and updates inventory.
// Called by CapUse after it prepares the player and capsule IDs.
static int CapUseSelect(CAP_USE_WORK *useWork)
{
    HuVecF playerPos;
    HuVecF capsulePos;
    HuVecF playerAnimPos;
    int capsuleObjId;
    int choiceWindowId;
    int previousObjId;
    int previousSelectionResult;
    int animFrame;
    int capsuleSlot;
    float animProgress;
    float capsuleScale;

    mbCapSelectResultGet(useWork->playerNo, &previousObjId,
        &previousSelectionResult);
    mbCapSelectResultReset(useWork->playerNo);
    if (previousObjId != MB_MODEL_NONE) {
        capsuleObjId = previousObjId;
    } else {
        capsuleObjId = mbCapObjCreate(useWork->capsuleNo, FALSE);
    }
    mbPlayerPosGet(useWork->playerNo, &playerPos);
    capsulePos = playerPos;
    capsulePos.y += 250.0f;
    if (previousObjId != MB_MODEL_NONE) {
        mbObjPosGet(capsuleObjId, &capsulePos);
        // Keep the existing height and depth, but align the capsule with the player.
        capsulePos.x = playerPos.x;
    }
    mbObjPosSet(capsuleObjId, capsulePos.x, capsulePos.y, capsulePos.z);
    mbObjLayerSet(capsuleObjId, 4);
    mbObjAttrSet(capsuleObjId, HU3D_MOTATTR_LOOP);
    if (previousObjId == MB_MODEL_NONE) {
        mbObjDispSet(capsuleObjId, TRUE);
        animFrame = 0;
        do {
            animFrame++;
            animProgress = (float)animFrame / 18.0f;
            if (animProgress > 1.0f) {
                animProgress = 1.0f;
            }
            mbPlayerPosGet(useWork->playerNo, &playerAnimPos);
            playerAnimPos.y += 100.0 +
                (150.0 * sin((M_PI * (90.0f * animProgress)) / 180.0));
            mbObjPosSetV(capsuleObjId, &playerAnimPos);
            capsuleScale = (double)1.2f *
                sin((M_PI * (90.0f * animProgress)) / 180.0);
            mbObjScaleSet(capsuleObjId, capsuleScale, capsuleScale,
                capsuleScale);
            HuPrcVSleep();
        } while (!mbCameraMoveCheck() || animProgress < 1.0f);
    }
    if (!GwPlayer[useWork->playerNo].comF) {
        // Later prompt calls use the top window, so this returned ID is unused.
        choiceWindowId = mbWinCreateChoice(
            MBWIN_TYPE_EVENT,
            MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_USE_CHOICE), -1,
            0);
        mbWinTopInsertMesSet(mbCapUseMesGet(useWork->capsuleNo), 0);
        // This CPU-player check is unreachable after the outer non-CPU test.
        if (GwPlayer[useWork->playerNo].comF) {
            CapComChoiceSet(0);
        }
        mbWinTopWait();
        if (mbWinTopChoiceGet() != 0 || mbWinTopChoiceGet() == -1) {
            if (previousObjId == MB_MODEL_NONE) {
                for (animFrame = 0; animFrame <= 18.0f; animFrame++) {
                    animProgress = 1.0f - ((float)animFrame / 18.0f);
                    if (animProgress < 0.0f) {
                        animProgress = 0.0f;
                    }
                    mbPlayerPosGet(useWork->playerNo, &playerAnimPos);
                    playerAnimPos.y += 100.0 + (150.0 *
                        sin((M_PI * (90.0f * animProgress)) / 180.0));
                    mbObjPosSetV(capsuleObjId, &playerAnimPos);
                    capsuleScale = (double)1.2f *
                        sin((M_PI * (90.0f * animProgress)) / 180.0);
                    mbObjScaleSet(capsuleObjId, capsuleScale, capsuleScale,
                        capsuleScale);
                    HuPrcVSleep();
                }
            } else {
                mbCapSelectResultSet(useWork->playerNo, previousObjId,
                    previousSelectionResult);
                capsuleObjId = MB_MODEL_NONE;
            }
            if (capsuleObjId != MB_MODEL_NONE) {
                mbCapObjKill(capsuleObjId);
            }
            return FALSE;
        }
    }
    if (!capsuleUseRemoveOnF) {
        mbPlayerCapsuleUseSet(useWork->capsuleNo);
        capsuleSlot = mbPlayerCapsuleFind(useWork->playerNo, useWork->capsuleNo);
        if (capsuleSlot != -1) {
            mbPlayerCapsuleRemove(useWork->playerNo, capsuleSlot);
        }
    }
    if (previousSelectionResult == -1) {
        previousSelectionResult = 0;
    }
    mbCapSelectResultSet(useWork->playerNo, capsuleObjId,
        previousSelectionResult);
    return TRUE;
}

// Starts the capsule-use effect as a child of the main board process.
// Called by capsule event and movement routines during capsule use.
BOOL mbCapEffUseCreate(int playerNo, int capsuleNo)
{
    CAP_EFF_USE_WORK *effectWork;
    CAP_EFF_USE_WORK *allocatedWork;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    capsuleUseEffProc[playerNo] =
        HuPrcChildCreate(CapEffUse, 8196, 24576, 0, mbMainProc);
    allocatedWork = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAP_EFF_USE_WORK), HU_MEMNUM_OVL);
    capsuleUseEffProc[playerNo]->property = effectWork = allocatedWork;
    memset(effectWork, 0, sizeof(CAP_EFF_USE_WORK));
    effectWork->playerNo = playerNo;
    effectWork->capsuleNo = capsuleNo;
    capsuleUseEffMode[playerNo] = 0;
    capsuleUseEffPos[playerNo].x = capsuleUseEffPos[playerNo].y =
        capsuleUseEffPos[playerNo].z = 0.0f;
    HuPrcDestructorSet2(capsuleUseEffProc[playerNo], CapEffUseKill);
    return TRUE;
}

// Returns the current visual-effect phase for a player's capsule.
int mbCapEffUseModeGet(int playerNo)
{
    return capsuleUseEffMode[playerNo];
}

// Returns the capsule-effect position after the effect has entered its active phase.
BOOL mbCapEffUsePosGet(int playerNo, HuVecF *pos)
{
    int mode = capsuleUseEffMode[playerNo];

    if (mode < 1) {
        return FALSE;
    }
    *pos = capsuleUseEffPos[playerNo];
    return TRUE;
}

// Starts the Wan Wan capsule effect at a supplied board position.
// Called by the board event that creates the Wan Wan capsule effect.
BOOL mbCapEffUseWanWanCreate(int playerNo, int capsuleValue, HuVecF *pos)
{
    CAP_EFF_USE_WORK *effectWork;
    int capsuleObjId;
    CAP_EFF_USE_WORK *allocatedWork;

    capsuleValue = mbCapValueTypeGet(capsuleValue);
    capsuleUseEffProc[playerNo] =
        HuPrcChildCreate(CapEffUse, 8196, 24576, 0, mbMainProc);
    allocatedWork = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(CAP_EFF_USE_WORK), HU_MEMNUM_OVL);
    effectWork = allocatedWork;
    capsuleUseEffProc[playerNo]->property = effectWork;
    memset(effectWork, 0, sizeof(CAP_EFF_USE_WORK));
    effectWork->playerNo = playerNo;
    effectWork->capsuleNo = capsuleValue;
    capsuleUseEffMode[playerNo] = 0;
    capsuleUseEffPos[playerNo].x = capsuleUseEffPos[playerNo].y =
        capsuleUseEffPos[playerNo].z = 0.0f;
    capsuleObjId = mbCapObjCreate(capsuleValue, FALSE);
    mbObjPosSetV(capsuleObjId, pos);
    mbObjDispSet(capsuleObjId, FALSE);
    mbCapSelectResultSet(effectWork->playerNo, capsuleObjId, FALSE);
    HuPrcDestructorSet2(capsuleUseEffProc[playerNo], CapEffUseKill);
    return TRUE;
}

// Runs the use-effect process started by the capsule-use entry points.
// Raises a new capsule, bursts rays and sparkles, then hides and releases it.
static void CapEffUse(void)
{
    CAP_EFF_USE_WORK *work = HuPrcCurrentGet()->property;
    OMOBJ *rayObj;
    OMOBJ *masuHitObj;
    OMOBJ *glowObj;
    HuVecF playerPos;
    HuVecF capPos;
    HuVecF rot;
    HuVecF rot2;
    HuVecF pos;
    HuVecF vel;
    HuVecF tempPos;
    HuVecF glowPos;
    HuVecF glowVel;
    GXColor color;
    Mtx mtx;
    int capObjId;
    int oldObjId;
    int result;
    int time;
    int i;
    int j;
    float scale;
    float t;
    float angle;
    GXColor glowColor;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;

    rayObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    masuHitObj = mbev_CapEffMasuHitCreate();
    glowObj = mbev_CapEffGlowCreate();
    mbCapSelectResultGet(work->playerNo, &oldObjId, &result);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        capObjId = oldObjId;
    } else {
        capObjId = mbCapObjCreate(work->capsuleNo, FALSE);
    }
    mbPlayerPosGet(work->playerNo, &playerPos);
    capPos = playerPos;
    capPos.y += 250.0f;
    // A selection preview keeps its height and depth, but follows the player's X position.
    if (oldObjId != MB_MODEL_NONE) {
        mbObjPosGet(capObjId, &capPos);
        capPos.x = playerPos.x;
    }
    mbObjPosSet(capObjId, capPos.x, capPos.y, capPos.z);
    mbObjLayerSet(capObjId, 4);
    mbObjAttrSet(capObjId, HU3D_MOTATTR_LOOP);
    mbev_CapEffRayTransformSet(rayObj, &capPos, NULL, NULL);
    mbev_CapEffMasuHitTransformSet(masuHitObj, &capPos, NULL, NULL);
    if (oldObjId == MB_MODEL_NONE) {
        mbObjDispSet(capObjId, TRUE);
        i = 0;
        do {
            i++;
            t = (float)i / 18.0f;
            if (t > 1.0f) {
                t = 1.0f;
            }
            mbPlayerPosGet(work->playerNo, &tempPos);
            tempPos.y += 100.0 + (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
            mbObjPosSetV(capObjId, &tempPos);
            scale = (double)1.2f *
                sin((M_PI * (90.0f * t)) / 180.0);
            mbObjScaleSet(capObjId, scale, scale, scale);
            HuPrcVSleep();
        } while (!mbCameraMoveCheck() || t < 1.0f);
    }
    capsuleUseEffMode[work->playerNo] = 1;
    capsuleUseEffPos[work->playerNo] = capPos;
    mbAudFXPlay(MSM_SE_BOARD_30);
    time = 0;
    while ((float)time < 30.0f) {
        t = (float)time / 30.0f;
        scale = cos((M_PI * (90.0f * t)) / 180.0);
        for (j = 0; j < 3; j++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * MBCapsuleEffRandF();
            rot.y = 360.0f * MBCapsuleEffRandF();
            rot.z = 360.0f * MBCapsuleEffRandF();
            rot2.x = rot.x + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.y = rot.y + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            rot2.z = rot.z + (45.0f * (-0.5f + MBCapsuleEffRandF()));
            mbev_CapEffRayAdd(rayObj, &pos, &rot, &rot2,
                200.0f * (1.0f + (0.25f * MBCapsuleEffRandF())),
                (int)(15.0f + (5.0f * MBCapsuleEffRandF())));
        }
        mbev_CapEffRayAlphaSet(rayObj, scale);
        for (j = 0; j < 5; j++) {
            pos.x = pos.y = pos.z = 0.0f;
            rot.x = 360.0f * MBCapsuleEffRandF();
            rot.y = 360.0f * MBCapsuleEffRandF();
            rot.z = 360.0f * MBCapsuleEffRandF();
            rot2.x = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.y = 360.0f * (-0.5f + MBCapsuleEffRandF());
            rot2.z = 360.0f * (-0.5f + MBCapsuleEffRandF());
            mbev_CapEffMasuHitAdd(masuHitObj, &pos, &rot, &rot2,
                100.0f * scale * (1.5f + MBCapsuleEffRandF()),
                25.0f * scale *
                    (1.0f + (0.5f * MBCapsuleEffRandF())),
                (int)(20.0f + (10.0f * MBCapsuleEffRandF())));
        }
        HuPrcVSleep();
        time++;
    }
    mtxRot(mtx, 0.0f, 0.0f, 20.0f);
    for (time = 0; time < 256; time++) {
        pos.x =
            capPos.x + (75.0f * (-0.5f + MBCapsuleEffRandF()));
        pos.y =
            capPos.y + (75.0f * (-0.5f + MBCapsuleEffRandF()));
        pos.z = capPos.z + CAP_EFF_GLOW_Z_OFFSET;
        angle = 360.0f * MBCapsuleEffRandF();
        scale = 5.0f * (0.5f + MBCapsuleEffRandF());
        vel.x = scale * sin((M_PI * angle) / 180.0);
        vel.y = scale * (0.3f * (-0.5f + MBCapsuleEffRandF()));
        vel.z = scale * cos((M_PI * angle) / 180.0);
        PSMTXMultVec(mtx, &vel, &vel);
        mbev_CapEffColorSet(&color, mbRandMod(CAPSULE_EFF_COLOR_RANGE));
        glowColor = color;
        glowColorP = &glowColor;
        glowVel = vel;
        glowVelP = &glowVel;
        glowPos = pos;
        glowPosP = &glowPos;
        mbev_CapEffGlowAdd(glowObj, glowPosP, glowVelP,
            (int)(60.0f * (0.4f + (0.5f * MBCapsuleEffRandF()))),
            100.0f * (0.3f + (0.2f * MBCapsuleEffRandF())),
            5.0f * (-0.5f + MBCapsuleEffRandF()), 0.0f, glowColorP);
        // Split the large sparkle burst across two frames.
        if (time == 128) {
            HuPrcVSleep();
        }
    }
    capsuleUseEffMode[work->playerNo] = 2;
    capsuleUseEffPos[work->playerNo] = capPos;
    mbObjDispSet(capObjId, FALSE);
    for (time = 0; time < 20; time++) {
        t = (float)time / 20.0f;
        scale = cos((M_PI * (90.0f * t)) / 180.0);
        mbObjScaleSet(capObjId, scale, scale, scale);
        HuPrcVSleep();
    }
    capsuleObjId = MB_MODEL_NONE;
    while (mbev_CapEffGlowDispGet(glowObj)) {
        HuPrcVSleep();
    }
    if (capObjId != MB_MODEL_NONE) {
        mbCapObjKill(capObjId);
    }
    mbev_CapEffRayKill(rayObj);
    mbev_CapEffMasuHitKill(masuHitObj);
    mbev_CapEffGlowKill(glowObj);
    HuPrcEnd();
}

// The use-effect process destructor clears the player's effect status and work.
// It also requests cleanup of the capsule removal and highlight effects.
static void CapEffUseKill(void)
{
    CAP_EFF_USE_WORK *work = HuPrcCurrentGet()->property;

    CapEffRemoveAddDestroy();
    CapEffHiliteKill();
    capsuleUseEffProc[work->playerNo] = NULL;
    capsuleUseEffMode[work->playerNo] = -1;
    HuMemDirectFree(work);
}

BOOL mbCapPlayerThrowCheck(void);

// Called after destination selection to start a player's throw and wait for its process.
void mbCapPlayerThrow(int playerNo, int masuId, int capsuleNo)
{
    CAP_PLAYER_THROW_WORK *work;
    CAP_PLAYER_THROW_WORK *workData;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    capsulePlayerThrowProc = HuPrcChildCreate(CapPlayerThrow, 8196, 24576, 0, mbMainProc);
    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAP_PLAYER_THROW_WORK), HU_MEMNUM_OVL);
    work = workData;
    capsulePlayerThrowProc->property = work;
    memset(work, 0, sizeof(CAP_PLAYER_THROW_WORK));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->capObjId0 = MB_MODEL_NONE;
    work->capObjId1 = MB_MODEL_NONE;
    work->objColorId = MB_MODEL_NONE;
    work->time = 0;
    HuPrcDestructorSet2(capsulePlayerThrowProc, CapPlayerThrowKill);
    while (mbCapPlayerThrowCheck() == FALSE) {
        HuPrcVSleep();
    }
}

// The throw entry point polls this until its process destructor clears the process.
BOOL mbCapPlayerThrowCheck(void)
{
    if (capsulePlayerThrowProc) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// Board setup supplies a hidden model for throw-path collision; MB_MODEL_NONE disables it.
// The return value is unset on the disable path, whose callers ignore it.
int mbCapThrowColCreate(int dataNum)
{
    int collisionObjId;

    if (dataNum != MB_MODEL_NONE) {
        collisionObjId = (s16)mbObjCreate(dataNum, NULL, FALSE);
        ((void (*)(int, BOOL))mbObjDispSet)(collisionObjId, FALSE);
        capsuleColObjId = collisionObjId;
        CapColMdlIdGet();
    } else {
        capsuleColObjId = MB_MODEL_NONE;
        CapColKill();
    }
    return collisionObjId;
}

// Exposes the board-model segment collision test to capsule throw callers.
BOOL mbCapThrowColCheck(HuVecF *segmentStart, HuVecF *segmentEnd, HuVecF *hitPos)
{
    BOOL hitF = CapColCheck(segmentStart, segmentEnd, hitPos);

    return hitF;
}

// The player's throw moves the capsule into the hand, selects and plays an arc,
// places the capsule, restores the player and camera, and removes it from
// inventory unless capsuleUseRemoveOnF is set.
static void CapPlayerThrow(void)
{
    CAP_PLAYER_THROW_WORK *work;
    int seNo;
    int playerObjId;
    int capsuleSlot;
    int handTime;
    int playerModelId;
    int coinNum;
    int motionTime;
    HuVecF effPos;
    HuVecF playerRot;
    HuVecF mdl2Pos;
    HuVecF dir;
    HuVecF dirOrig;
    HuVecF pos;
    HuVecF vel;
    HuVecF playerPos;
    HuVecF playerPosOrig;
    HuVecF capPos;
    HuVecF handPos;
    float x[3];
    float y[3];
    float z[3];
    HuVecF throwOut;
    HuVecF glowPos;
    HuVecF glowVel;
    Mtx rotMtx;
    Mtx objMtx;
    Mtx handMtx;
    float time;
    float t;
    float arc;
    float scale;
    float mag;
    GXColor color;
    float magTemp;
    GXColor glowColor;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;

    work = HuPrcCurrentGet()->property;
    if (capsuleThrowHook) {
        capsuleThrowHook(TRUE);
    }
    CapEffThrowMasuCreate(work->masuId, work->capsuleNo);
    work->jumpMotId =
        mbPlayerMotionCreate(work->playerNo, CHARMOT_HSF_c000m1_431);
    HuPrcVSleep();
    if (capsuleObjId != MB_MODEL_NONE) {
        work->capObjId0 = capsuleObjId;
    } else {
        work->capObjId0 =
            mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, FALSE);
    }
    mbPlayerPosGet(work->playerNo, &pos);
    mbObjPosSet(work->capObjId0, pos.x, pos.y + 250.0f, pos.z);
    mbObjScaleSet(work->capObjId0, 1.2f, 1.2f, 1.2f);
    mbObjLayerSet(work->capObjId0, 4);
    work->capObjId1 = mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, FALSE);
    mbObjDispSet(work->capObjId1, FALSE);
    while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo))
        != MB_MODEL_NONE) {
        HuPrcVSleep();
    }
    playerObjId = mbPlayerObjIDGet(work->playerNo);
    playerModelId = mbObjModelIDGet(playerObjId);
    motionTime = (int)mbObjMotionTimeGet(playerObjId);
    mbPlayerRotGet(work->playerNo, &playerRot);
    mbPlayerPosGet(work->playerNo, &work->pos);
    mbMasuPosGet(work->masuId, &work->masuPos);
    PSVECSubtract(&work->masuPos, &work->pos, &dir);
    mbPlayerRotSet(work->playerNo, 0.0f,
        (float)((atan2(dir.x, dir.z) / M_PI) * 180.0), 0.0f);
    // Sample the future release pose to find the hand position before starting the throw.
    mbPlayerMotionSet(work->playerNo, work->jumpMotId, HU3D_MOTATTR_NONE);
    mbPlayerMotionTimeSet(
        work->playerNo, 0.35f * mbPlayerMotionMaxTimeGet(work->playerNo));
    Hu3DMotionCalc(playerModelId);
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    work->pos.x = handMtx[0][3];
    work->pos.y = handMtx[1][3];
    work->pos.z = handMtx[2][3];
    mbPlayerMotionSet(work->playerNo, 1, HU3D_MOTATTR_LOOP);
    mbObjMotionTimeSet(playerObjId, (float)motionTime);
    mbPlayerRotSetV(work->playerNo, &playerRot);
    Hu3DMotionCalc(playerModelId);
    HuPrcVSleep();
    mbMasuPosGet(work->masuId, &work->masuPos);
    work->masuPos.y += 20.0f;
    PSVECSubtract(&work->masuPos, &work->pos, &dir);
    mag = PSVECMag(&dir);
    work->yOfs = 50.0f + (0.1f * mag);
    if (work->yOfs > 700.0f) {
        work->yOfs = 700.0f;
    }
    playerPosOrig = work->pos;
    playerPos = playerPosOrig;
    x[0] = work->pos.x;
    y[0] = work->pos.y;
    z[0] = work->pos.z;
    x[1] = work->pos.x;
    y[1] = work->pos.y;
    z[1] = work->pos.z;
    x[2] = work->masuPos.x;
    y[2] = work->masuPos.y;
    z[2] = work->masuPos.z;
    CapEffThrowCreate(
        work->playerNo, x, y, z, work->yOfs, work->masuId);
    if (!GwPlayer[work->playerNo].comF) {
        mbWipeDissolveFadeInTime(15);
    }
    handTime = 45;
    mbObjPosGet(work->capObjId0, &capPos);
    Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    handPos.x = handMtx[0][3];
    handPos.y = handMtx[1][3];
    handPos.z = handMtx[2][3];
    mbAudFXPlay(MSM_SE_BOARD_36);
    for (time = 0.0f; time < handTime; time++) {
        Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
        Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
            CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0),
            handMtx);
        handPos.x = handMtx[0][3];
        handPos.y = handMtx[1][3];
        handPos.z = handMtx[2][3];
        t = sin((M_PI * (90.0f * (time / handTime))) / 180.0);
        arc = 100.0f * sin((M_PI * (180.0f * t)) / 180.0);
        pos.x = capPos.x + (t * (handPos.x - capPos.x))
            + (arc * sin((M_PI * (180.0f - (270.0f * t))) / 180.0));
        pos.y = capPos.y + (t * (handPos.y - capPos.y));
        pos.z = capPos.z + (t * (handPos.z - capPos.z))
            + (arc * cos((M_PI * (180.0f - (270.0f * t))) / 180.0));
        scale = 0.35f + (0.85f * cos((M_PI * (90.0f * t)) / 180.0));
        mbObjPosSetV(work->capObjId0, &pos);
        mbObjScaleSet(work->capObjId0, scale, scale, scale);
        HuPrcVSleep();
    }
    mbObjHookSet(mbPlayerObjIDGet(work->playerNo),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0),
        work->capObjId0);
    mbPlayerRotateStart(work->playerNo,
        (s16)((atan2(dir.x, dir.z) / M_PI) * 180.0), 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    do {
        HuPrcVSleep();
    } while (!CapEffThrowCheck(&pos, &work->maxTime));
    CapEffThrowKill();
    x[1] = pos.x;
    y[1] = pos.y;
    z[1] = pos.z;
    work->time = 0;
    mbPlayerMotionShiftSet(work->playerNo, work->jumpMotId, 0.0f, 8.0f,
        HU3D_MOTATTR_NONE);
    mbAudFXPlay(MSM_SE_BOARD_25);
    while (mbObjMotionShiftIDGet(mbPlayerObjIDGet(work->playerNo))
        != MB_MODEL_NONE) {
        HuPrcVSleep();
    }
    time = mbPlayerMotionMaxTimeGet(work->playerNo);
    while (mbPlayerMotionTimeGet(work->playerNo) <= (0.35f * time)) {
        HuPrcVSleep();
    }
    Hu3DMotionCalc(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)));
    Hu3DModelObjMtxGet(mbObjModelIDGet(mbPlayerObjIDGet(work->playerNo)),
        CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0), handMtx);
    x[0] = work->pos.x = handMtx[0][3];
    y[0] = work->pos.y = handMtx[1][3];
    z[0] = work->pos.z = handMtx[2][3];
    if (work->capObjId0 != MB_MODEL_NONE) {
        mbObjHookObjReset(mbPlayerObjIDGet(work->playerNo),
            CharModelItemHookGet(GwPlayer[work->playerNo].charNo, 4, 0));
        mbObjDispSet(work->capObjId0, FALSE);
    }
    capsuleObjId = MB_MODEL_NONE;
    work->objColorId = mbCapObjColorCreate(work->capsuleNo, FALSE);
    mbPlayerPosGet(work->playerNo, &pos);
    mbCapObjColorPosSet(
        work->objColorId, pos.x, 100.0f + pos.y, pos.z);
    mbCapObjColorScaleSet(work->objColorId, 1.0f, 1.0f, 1.0f);
    mbCapObjColorLayerSet(work->objColorId, 4);
    CapEffTrailAdd(&work->pos, work->capsuleNo);
    seNo = mbAudFXPlay(MSM_SE_BOARD_26);
    do {
        work->time++;
        t = (float)work->time / (float)work->maxTime;
        PSVECSubtract(&work->masuPos, &work->pos, &dir);
        PSVECScale(&dir, &dir, t);
        PSVECAdd(&work->pos, &dir, &effPos);
        mdl2Pos = effPos;
        CapThrowCameraCalc(t, x, y, z, &throwOut, 3);
        effPos.x = throwOut.x;
        effPos.y = throwOut.y;
        effPos.z = throwOut.z;
        PSVECSubtract(&effPos, &mdl2Pos, &pos);
        PSVECScale(&pos, &pos, 0.75f);
        PSVECAdd(&mdl2Pos, &pos, &mdl2Pos);
        mbCapObjColorPosSet(
            work->objColorId, effPos.x, effPos.y, effPos.z);
        mbCapObjColorScaleSet(work->objColorId, 0.5f, 0.5f, 0.5f);
        PSVECSubtract(&work->masuPos, &work->pos, &dir);
        dirOrig = dir;
        dir.x = dirOrig.z;
        dir.y = 0.0f;
        dir.z = -dirOrig.x;
        if ((magTemp = PSVECMag(&dir)) <= 0.0f) {
            dir.z = 1.0f;
        }
        PSMTXRotAxisRad(rotMtx, &dir, 0.1f);
        mbCapObjColorMtxGet(work->objColorId, &objMtx);
        PSMTXConcat(rotMtx, objMtx, objMtx);
        mbCapObjColorMtxSet(work->objColorId, &objMtx);
        CapEffTrailPosSet(&effPos);
        mbObjPosSet(work->capObjId1, mdl2Pos.x, mdl2Pos.y, mdl2Pos.z);
        // This hidden model follows a softened arc solely as the camera focus.
        mbObjDispSet(work->capObjId1, FALSE);
        mbCameraFocusObjSet(work->capObjId1);
        if (capsuleThrowGlowOMObj != NULL) {
            pos.x = effPos.x +
                ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            pos.y = effPos.y +
                ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            pos.z = effPos.z +
                ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            vel.x = vel.y = vel.z = 0.0f;
            color = capsulePlayerThrowColorTbl[
                mbCapColorGet(work->capsuleNo)];
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            glowColor = color;
            glowColorP = &glowColor;
            glowVel = vel;
            glowVelP = &glowVel;
            glowPos = pos;
            glowPosP = &glowPos;
            mbev_CapEffGlowAdd(capsuleThrowGlowOMObj, glowPosP, glowVelP,
                (int)(60.0f * (1.0f + MBCapsuleEffRandF())),
                100.0f *
                    (0.2f + (0.025f * MBCapsuleEffRandF())),
                0.0f, 0.025f, glowColorP);
        }
        HuPrcVSleep();
    } while (work->time < work->maxTime);
    if (seNo != MSM_SENO_NONE) {
        mbAudFXStop(seNo);
    }
    seNo = MSM_SENO_NONE;
    mbAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->masuPos;
    effPos.y -= 10000.0f;
    CapEffTrailPosSet(&effPos);
    mbObjDispSet(work->capObjId0, FALSE);
    mbObjDispSet(work->capObjId1, FALSE);
    mbCapObjColorDispSet(work->objColorId, FALSE);
    omVibrate(work->playerNo, 20, 4, 4);
    coinNum = CapEffThrowMasu(
        work->masuId, work->capsuleNo, work->playerNo, TRUE);
    CapEffThrowMasuWait(coinNum);
    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        mbTutorialCall(18);
    }
    CapThrowEndWin(work->masuId, work->capsuleNo);
    mbev_CapPlayerMotShiftWait(
        work->playerNo, 1, HU3D_MOTATTR_LOOP, TRUE);
    mbPlayerRotateStart(work->playerNo, 0, 15);
    while (!mbPlayerRotateCheck(work->playerNo)) {
        HuPrcVSleep();
    }
    mbCameraFocusPlayerSet(work->playerNo);
    mbCameraMoveOnSet(FALSE);
    if (capsuleThrowHook) {
        capsuleThrowHook(FALSE);
    }
    mbCameraMoveWait();
    mbCameraMoveOnSet(TRUE);
    mbev_CapBonusCoinCall(
        work->playerNo, work->capsuleNo, coinNum, TRUE);
    if (!capsuleUseRemoveOnF) {
        mbPlayerCapsuleUseSet(work->capsuleNo);
        capsuleSlot =
            mbPlayerCapsuleFind(work->playerNo, work->capsuleNo);
        if (capsuleSlot != -1) {
            mbPlayerCapsuleRemove(work->playerNo, capsuleSlot);
        }
    }
    if (work->jumpMotId != MB_MODEL_NONE && !mbExitCheck()) {
        mbPlayerMotionKill(work->playerNo, work->jumpMotId);
    }
    work->jumpMotId = MB_MODEL_NONE;
    HuPrcEnd();
}

// The throw-process destructor releases its path search, capsule models, and work record.
static void CapPlayerThrowKill(void)
{
    CAP_PLAYER_THROW_WORK *work = HuPrcCurrentGet()->property;

    CapEffThrowKill();
    mbCapObjKill(work->capObjId0);
    mbCapObjKill(work->capObjId1);
    mbCapObjColorKill(work->objColorId);
    HuMemDirectFree(work);
    capsulePlayerThrowProc = NULL;
}

// Player and scripted throws start a render-hook search for a usable three-point arc.
// The supplied player and space are unused; player collision starts disabled.
static void CapEffThrowCreate(int playerNo, float *x, float *y, float *z,
    float yOfs, int masuId)
{
    HU3D_MODEL *model;
    CAP_EFF_THROW_WORK *work;
    int boardNoTemp;
    int boardNo;

    capEffThrowMdlId = Hu3DHookFuncCreate(CapEffThrowHook);
    Hu3DModelCameraSet(capEffThrowMdlId, 1);
    Hu3DModelLayerSet(capEffThrowMdlId, 7);
    model = &Hu3DData[capEffThrowMdlId];
    work = HuMemDirectMallocNum(HEAP_MODEL, sizeof(*work), model->mallocNo);
    model->hookData = work;
    memset(work, 0, sizeof(*work));
    work->modelId = capEffThrowMdlId;
    work->playerNo = -2;
    work->maxTime = 0;
    work->endF = FALSE;
    work->minNo = 0;
    work->no = work->minNo;
    work->initF = FALSE;
    work->startPos.x = x[0];
    work->startPos.y = y[0];
    work->startPos.z = z[0];
    work->endPos.x = x[2];
    work->endPos.y = y[2];
    work->endPos.z = z[2];
    work->pos.x = work->pos.y = work->pos.z = 0.0f;
    work->yOfs = yOfs;
    work->tick = 0;
    boardNoTemp = GwSystem.boardNo;
    boardNo = boardNoTemp;
    if (boardNo < 0) {
        boardNo = 0;
    } else if (boardNo >= 11) {
        boardNo = 10;
    }
    work->delay = capsulePlayerThrowDelayTbl[boardNo];
    work->x[0] = x[0];
    work->y[0] = y[0];
    work->z[0] = z[0];
    work->x[1] = x[1];
    work->y[1] = y[1];
    work->z[1] = z[1];
    work->x[2] = x[2];
    work->y[2] = y[2];
    work->z[2] = z[2];
}

// The model render hook tests candidate throw arcs in slices of the board's tick budget.
// It keeps the first clear arc, or falls back to the first candidate after all fail.
static void CapEffThrowHook(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFF_THROW_WORK *work = modelP->hookData;
    int i;
    u32 tick;
    HuVecF pos;
    HuVecF dir;
    HuVecF out;
    float mag;
    float t;
    float angle;

    if (work->endF) {
        return;
    }
    if (!work->initF) {
        PSVECSubtract(&work->endPos, &work->startPos, &pos);
        PSVECScale(&pos, &pos, capsulePlayerThrowFrameTbl[work->no].speed);
        PSVECAdd(&pos, &work->startPos, &pos);
        if (pos.y < work->startPos.y) {
            pos.y = work->startPos.y;
        } else if (pos.y < work->endPos.y) {
            pos.y = work->endPos.y;
        }
        pos.y += work->yOfs + capsulePlayerThrowFrameTbl[work->no].yOfs;
        if (0.0f != capsulePlayerThrowFrameTbl[work->no].radius) {
            PSVECSubtract(&work->endPos, &work->startPos, &dir);
            if (PSVECMag(&dir) > 0.0f) {
                PSVECNormalize(&dir, &dir);
            }
            angle = ((atan2(dir.x, dir.z) / M_PI) * 180) + 90;
            dir.x = sin((M_PI * angle) / 180) * capsulePlayerThrowFrameTbl[work->no].radius;
            dir.z = cos((M_PI * angle) / 180) * capsulePlayerThrowFrameTbl[work->no].radius;
            pos.x += dir.x;
            pos.z += dir.z;
        } else if (0.0f != capsulePlayerThrowFrameTbl[work->no].dir) {
            PSVECSubtract(&work->endPos, &work->startPos, &dir);
            mag = PSVECMag(&dir);
            if (PSVECMag(&dir) > 0.0f) {
                PSVECNormalize(&dir, &dir);
            }
            t = dir.z;
            dir.z = (dir.x * mag) * capsulePlayerThrowFrameTbl[work->no].dir;
            dir.x = (t * mag) * capsulePlayerThrowFrameTbl[work->no].dir;
            pos.x += dir.x;
            pos.z += dir.z;
        }
        work->x[1] = pos.x;
        work->y[1] = pos.y;
        work->z[1] = pos.z;
        CapThrowCameraSet(work->x, work->y, work->z, 3);
        PSVECSubtract(&pos, &work->startPos, &dir);
        mag = PSVECMag(&dir);
        PSVECSubtract(&work->endPos, &pos, &dir);
        mag += PSVECMag(&dir);
        work->maxTime = mag / 25;
        if (work->maxTime < 24.0f) {
            work->maxTime = 24;
        }
        work->pos = work->startPos;
        work->initF = TRUE;
    }
    work->tick = OSGetTick();
    // initF also holds the current path sample, resuming where the previous hook stopped.
    for (i = 0; work->initF < (0.5f * work->maxTime); i++) {
        t = work->initF / (0.5f * work->maxTime);
        CapThrowCameraCalc(t, work->x, work->y, work->z, &out, 3);
        pos.x = out.x;
        pos.y = out.y;
        pos.z = out.z;
        if (CapColCheck(&work->pos, &pos, &dir)) {
            break;
        }
        if (work->playerNo != -2 && CapColExec(work->playerNo, &work->pos, &pos, &dir)) {
            break;
        }
        work->pos = pos;
        work->initF++;
        tick = OSGetTick();
        if ((tick - work->tick) > work->delay) {
            return;
        }
    }
    if (work->initF >= (0.5f * work->maxTime)) {
        work->endF = TRUE;
        return;
    }
    work->initF = FALSE;
    work->no++;
    if (work->no < 48) {
        return;
    }
    if (work->playerNo != -2) {
        work->initF = FALSE;
        work->no = 0;
        work->playerNo = -2;
        return;
    }
    work->no = 0;
    PSVECSubtract(&work->endPos, &work->startPos, &pos);
    PSVECScale(&pos, &pos, capsulePlayerThrowFrameTbl[work->no].speed);
    PSVECAdd(&pos, &work->startPos, &pos);
    if (pos.y < work->startPos.y) {
        pos.y = work->startPos.y;
    } else if (pos.y < work->endPos.y) {
        pos.y = work->endPos.y;
    }
    pos.y += work->yOfs + capsulePlayerThrowFrameTbl[work->no].yOfs;
    work->x[1] = pos.x;
    work->y[1] = pos.y;
    work->z[1] = pos.z;
    CapThrowCameraSet(work->x, work->y, work->z, 3);
    PSVECSubtract(&pos, &work->startPos, &dir);
    mag = PSVECMag(&dir);
    PSVECSubtract(&work->endPos, &pos, &dir);
    mag += PSVECMag(&dir);
    work->maxTime = mag / 25;
    if (work->maxTime < 24.0f) {
        work->maxTime = 24;
    }
    work->endF = TRUE;
}

// The player throw polls for a finished path search and receives its bend and duration.
static BOOL CapEffThrowCheck(HuVecF *pos, int *maxTime)
{
    HU3D_MODEL *model;
    CAP_EFF_THROW_WORK *work;

    if (capEffThrowMdlId == MB_MODEL_NONE) {
        return FALSE;
    }
    model = &Hu3DData[capEffThrowMdlId];
    work = model->hookData;
    if (work->endF == FALSE) {
        return FALSE;
    }
    pos->x = work->x[1];
    pos->y = work->y[1];
    pos->z = work->z[1];
    *maxTime = work->maxTime;
    return TRUE;
}

// Throw completion and its process destructor remove the render-hook path search.
static void CapEffThrowKill(void)
{
    if (capEffThrowMdlId != MB_MODEL_NONE) {
        Hu3DModelKill(capEffThrowMdlId);
    }
    capEffThrowMdlId = MB_MODEL_NONE;
}

// Before a player throws, creates the trail and impact effects for its destination space.
static void CapEffThrowMasuCreate(int masuId, int capsuleNo)
{
    HuVecF pos;
    HuVecF rot;

    if (masuId > 0) {
        mbMasuPosGet((s16)masuId, &pos);
        mbMasuRotGet((s16)masuId, &rot);
    }
    capsuleThrowGlowOMObj = mbev_CapEffGlowCreate();
    capsuleThrowRingOMObj = mbev_CapEffRingCreate();
    capsuleThrowRayOMObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    if (masuId > 0) {
        mbev_CapEffRayTransformSet(capsuleThrowRayOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuHitOMObj = mbev_CapEffMasuHitCreate();
    if (masuId > 0) {
        mbev_CapEffMasuHitTransformSet(
            capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuCoinOMObj = mbev_CapEffCoinCreate();
    CapEffTrailCreate(capsuleNo);
    CapEffCrackCreate();
}

// Player and automatic throws run this landing burst, placing the capsule at frame 20.
// Returns the bonus-coin count; -1 suppresses coins outside eligible party throws.
static int CapEffThrowMasu(int masuId, int capsuleNo, int playerNo, BOOL bonusF)
{
    HuVecF pos;
    HuVecF rot;
    HuVecF rayRot;
    HuVecF rayRot2;
    HuVecF zeroPos;
    HuVecF vel;
    HuVecF ringStartPos;
    HuVecF ringStartRot;
    HuVecF ringStartVel;
    HuVecF ringBurstPos;
    HuVecF ringBurstRot;
    HuVecF ringBurstVel;
    GXColor color;
    int life = 0;
    GXColor ringStartColor;
    GXColor ringBurstColor;
    int coinLeft;
    int coinNum = 0;
    int coinDelay;
    int time;
    int i;
    float alpha;
    s16 ringColorRaw;
    s16 ringVelRaw;
    float ringVelF;
    s16 coinDelayRaw;
    float coinDelayF;
    s16 rayScaleRaw;
    float rayScaleF;
    s16 rayLifeRaw;
    float rayLifeF;
    s16 hitScaleRaw;
    float hitScaleF;
    s16 hitScaleYRaw;
    float hitScaleYF;
    s16 hitLifeRaw;
    float hitLifeF;
    s16 rayRotXRaw;
    float rayRotXF;
    float rayRotXOffset;
    s16 rayRotYRaw;
    float rayRotYF;
    float rayRotYOffset;
    s16 rayRotZRaw;
    float rayRotZF;
    float rayRotZOffset;
    s16 rayJitterXRaw;
    float rayJitterXF;
    float rayJitterXOffset;
    s16 rayJitterYRaw;
    float rayJitterYF;
    float rayJitterYOffset;
    s16 rayJitterZRaw;
    float rayJitterZF;
    float rayJitterZOffset;
    s16 hitRotXRaw;
    float hitRotXF;
    float hitRotXOffset;
    s16 hitRotYRaw;
    float hitRotYF;
    float hitRotYOffset;
    s16 hitRotZRaw;
    float hitRotZF;
    float hitRotZOffset;
    s16 hitRot2XRaw;
    float hitRot2XF;
    float hitRot2XOffset;
    s16 hitRot2YRaw;
    float hitRot2YF;
    float hitRot2YOffset;
    s16 hitRot2ZRaw;
    float hitRot2ZF;
    float hitRot2ZOffset;
    BOOL partyF;
    GXColor *ringStartColorP;
    HuVecF *ringStartVelP;
    HuVecF *ringStartRotP;
    HuVecF *ringStartPosP;
    GXColor *ringBurstColorP;
    HuVecF *ringBurstVelP;
    HuVecF *ringBurstRotP;
    HuVecF *ringBurstPosP;

    partyF = GwSystem.partyF;
    if (!partyF ||
        _CheckFlag(FLAG_BOARD_TUTORIAL) ||
        !bonusF) {
        coinNum = -1;
    } else {
        coinNum = mbCapBonusCoinNumGet(playerNo, capsuleNo);
    }
    coinLeft = coinNum;
    coinDelay = 30;
    zeroPos.x = zeroPos.y = zeroPos.z = 0.0f;
    mbMasuPosGet(masuId, &pos);
    mbMasuRotGet(masuId, &rot);
    CapEffCrackAdd(&pos, &rot);
    mbMasuPosGet(masuId, &pos);
    rot.x = -90.0f;
    rot.y = 0.0f;
    rot.z = 0.0f;
    vel.x = 2.0f;
    vel.y = 4.0f;
    vel.z = 100.0f * (1.0f + (0.25f * MBCapsuleEffRandF()));
    mbev_CapEffColorSet(&color, mbRandMod(CAPSULE_EFF_COLOR_RANGE));
    ringStartColor = color;
    ringStartColorP = &ringStartColor;
    ringStartVel = vel;
    ringStartVelP = &ringStartVel;
    ringStartRot = rot;
    ringStartRotP = &ringStartRot;
    ringStartPos = pos;
    ringStartPosP = &ringStartPos;
    mbev_CapEffRingAdd(capsuleThrowRingOMObj, ringStartPosP,
        ringStartRotP, ringStartVelP, 3, 15, 1, ringStartColorP);
    for (time = 0; (float)time <= 60.0f || coinLeft > 0; time++) {
        // Keep the fading effects at their last frame while any bonus coins remain.
        if ((float)time > 60.0f) {
            time = 60;
        }
        if (time == 20) {
            mbCapMasuCapsuleSet(masuId, capsuleNo, playerNo);
            mbAudFXPlay(MSM_SE_BOARD_28);
        }
        if (time == 10 || time == 20) {
            mbMasuPosGet(masuId, &pos);
            rot.x = -90.0f;
            rot.y = 0.0f;
            rot.z = 0.0f;
            vel.x = 2.0f;
            vel.y = 6.0f;
            CAP_EFF_RAND_NEXT();
            ringVelRaw = mbCapEffData[mbCapEffNum];
            ringVelF = (float)ringVelRaw * (1.0f / 32767.0f);
            vel.z = 100.0f * (1.0f + (0.25f * ringVelF));
            CAP_EFF_RAND_NEXT();
            ringColorRaw = mbCapEffData[mbCapEffNum];
            mbev_CapEffColorSet(&color, ringColorRaw);
            ringBurstColor = color;
            ringBurstColorP = &ringBurstColor;
            ringBurstVel = vel;
            ringBurstVelP = &ringBurstVel;
            ringBurstRot = rot;
            ringBurstRotP = &ringBurstRot;
            ringBurstPos = pos;
            ringBurstPosP = &ringBurstPos;
            mbev_CapEffRingAdd(capsuleThrowRingOMObj, ringBurstPosP,
                ringBurstRotP, ringBurstVelP, 3, 10, 2, ringBurstColorP);
        }
        coinDelay--;
        if (coinDelay <= 0 && coinLeft > 0) {
            mbMasuPosGet(masuId, &pos);
            mbev_CapEffCoinMultiAdd(capsuleThrowMasuCoinOMObj, &pos, 1);
            coinLeft--;
            CAP_EFF_RAND_NEXT();
            coinDelayRaw = mbCapEffData[mbCapEffNum];
            coinDelayF = (float)coinDelayRaw * (1.0f / 32767.0f);
            coinDelay = (int)(3.0f * coinDelayF);
        }
        if (masuId > 0) {
            mbMasuPosGet(masuId, &pos);
            mbMasuRotGet(masuId, &rot);
            mbev_CapEffRayTransformSet(
                capsuleThrowRayOMObj, &pos, &rot, NULL);
            mbev_CapEffMasuHitTransformSet(
                capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
        }
        alpha = cos((M_PI * (90.0f * ((float)time / 60.0f))) / 180.0);
        for (i = 0; (float)i < (2.0f * alpha); i++) {
            pos = zeroPos;
            CAP_EFF_RAND_NEXT();
            rayRotXRaw = mbCapEffData[mbCapEffNum];
            rayRotXF = (float)rayRotXRaw * (1.0f / 32767.0f);
            rayRotXOffset = rayRotXF - 0.5f;
            rayRot.x = 180.0f * rayRotXOffset;
            CAP_EFF_RAND_NEXT();
            rayRotYRaw = mbCapEffData[mbCapEffNum];
            rayRotYF = (float)rayRotYRaw * (1.0f / 32767.0f);
            rayRotYOffset = rayRotYF - 0.5f;
            rayRot.y = 360.0f * rayRotYOffset;
            CAP_EFF_RAND_NEXT();
            rayRotZRaw = mbCapEffData[mbCapEffNum];
            rayRotZF = (float)rayRotZRaw * (1.0f / 32767.0f);
            rayRotZOffset = rayRotZF - 0.5f;
            rayRot.z = 180.0f * rayRotZOffset;
            CAP_EFF_RAND_NEXT();
            rayJitterXRaw = mbCapEffData[mbCapEffNum];
            rayJitterXF = (float)rayJitterXRaw * (1.0f / 32767.0f);
            rayJitterXOffset = rayJitterXF - 0.5f;
            rayRot2.x = rayRot.x + (45.0f * rayJitterXOffset);
            CAP_EFF_RAND_NEXT();
            rayJitterYRaw = mbCapEffData[mbCapEffNum];
            rayJitterYF = (float)rayJitterYRaw * (1.0f / 32767.0f);
            rayJitterYOffset = rayJitterYF - 0.5f;
            rayRot2.y = rayRot.y + (45.0f * rayJitterYOffset);
            CAP_EFF_RAND_NEXT();
            rayJitterZRaw = mbCapEffData[mbCapEffNum];
            rayJitterZF = (float)rayJitterZRaw * (1.0f / 32767.0f);
            rayJitterZOffset = rayJitterZF - 0.5f;
            rayRot2.z = rayRot.z + (45.0f * rayJitterZOffset);
            CAP_EFF_RAND_NEXT();
            rayScaleRaw = mbCapEffData[mbCapEffNum];
            rayScaleF = (float)rayScaleRaw * (1.0f / 32767.0f);
            CAP_EFF_RAND_NEXT();
            rayLifeRaw = mbCapEffData[mbCapEffNum];
            rayLifeF = (float)rayLifeRaw * (1.0f / 32767.0f);
            mbev_CapEffRayAdd(capsuleThrowRayOMObj,
                &pos, &rayRot, &rayRot2,
                200.0f * (1.0f + rayScaleF),
                (int)(20.0f + (5.0f * rayLifeF)));
        }
        mbev_CapEffRayAlphaSet(capsuleThrowRayOMObj, alpha);
        for (i = 0; (float)i < (5.0f * alpha); i++) {
            pos = zeroPos;
            CAP_EFF_RAND_NEXT();
            hitRotXRaw = mbCapEffData[mbCapEffNum];
            hitRotXF = (float)hitRotXRaw * (1.0f / 32767.0f);
            hitRotXOffset = hitRotXF - 0.5f;
            rayRot.x = 360.0f * hitRotXOffset;
            CAP_EFF_RAND_NEXT();
            hitRotYRaw = mbCapEffData[mbCapEffNum];
            hitRotYF = (float)hitRotYRaw * (1.0f / 32767.0f);
            hitRotYOffset = hitRotYF - 0.5f;
            rayRot.y = 360.0f * hitRotYOffset;
            CAP_EFF_RAND_NEXT();
            hitRotZRaw = mbCapEffData[mbCapEffNum];
            hitRotZF = (float)hitRotZRaw * (1.0f / 32767.0f);
            hitRotZOffset = hitRotZF - 0.5f;
            rayRot.z = 360.0f * hitRotZOffset;
            CAP_EFF_RAND_NEXT();
            hitRot2XRaw = mbCapEffData[mbCapEffNum];
            hitRot2XF = (float)hitRot2XRaw * (1.0f / 32767.0f);
            hitRot2XOffset = hitRot2XF - 0.5f;
            rayRot2.x = 360.0f * hitRot2XOffset;
            CAP_EFF_RAND_NEXT();
            hitRot2YRaw = mbCapEffData[mbCapEffNum];
            hitRot2YF = (float)hitRot2YRaw * (1.0f / 32767.0f);
            hitRot2YOffset = hitRot2YF - 0.5f;
            rayRot2.y = 360.0f * hitRot2YOffset;
            CAP_EFF_RAND_NEXT();
            hitRot2ZRaw = mbCapEffData[mbCapEffNum];
            hitRot2ZF = (float)hitRot2ZRaw * (1.0f / 32767.0f);
            hitRot2ZOffset = hitRot2ZF - 0.5f;
            rayRot2.z = 360.0f * hitRot2ZOffset;
            CAP_EFF_RAND_NEXT();
            hitScaleRaw = mbCapEffData[mbCapEffNum];
            hitScaleF = (float)hitScaleRaw * (1.0f / 32767.0f);
            CAP_EFF_RAND_NEXT();
            hitScaleYRaw = mbCapEffData[mbCapEffNum];
            hitScaleYF = (float)hitScaleYRaw * (1.0f / 32767.0f);
            CAP_EFF_RAND_NEXT();
            hitLifeRaw = mbCapEffData[mbCapEffNum];
            hitLifeF = (float)hitLifeRaw * (1.0f / 32767.0f);
            mbev_CapEffMasuHitAdd(capsuleThrowMasuHitOMObj,
                &pos, &rayRot, &rayRot2,
                100.0f * alpha * (1.5f + hitScaleF),
                25.0f * alpha * (1.0f + (0.5f * hitScaleYF)),
                (int)(20.0f + (10.0f * hitLifeF)));
        }
        HuPrcVSleep();
    }
    return coinNum;
}

// The player throw waits for landing coins and releases all throw effects here.
// Its argument is the bonus count: only nonpositive values also wait for the glow to finish.
static BOOL CapEffThrowMasuWait(BOOL bonusCoinCount)
{
    HuPrcSleep(18);
    while (mbev_CapEffCoinNumGet(capsuleThrowMasuCoinOMObj) > 0) {
        HuPrcVSleep();
    }
    if (bonusCoinCount <= 0) {
        while (mbev_CapEffGlowDispGet(capsuleThrowGlowOMObj) != 0) {
            HuPrcVSleep();
        }
    }
    if (capsuleThrowGlowOMObj != NULL) {
        mbev_CapEffGlowKill(capsuleThrowGlowOMObj);
    }
    if (capsuleThrowRingOMObj != NULL) {
        mbev_CapEffRingKill(capsuleThrowRingOMObj);
    }
    if (capsuleThrowRayOMObj != NULL) {
        mbev_CapEffRayKill(capsuleThrowRayOMObj);
    }
    if (capsuleThrowMasuHitOMObj != NULL) {
        mbev_CapEffMasuHitKill(capsuleThrowMasuHitOMObj);
    }
    if (capsuleThrowMasuCoinOMObj != NULL) {
        mbev_CapEffCoinKill(capsuleThrowMasuCoinOMObj);
    }
    CapEffCrackKill();
    CapEffTrailKill();
    return TRUE;
}

void mbCapThrowHookSet(CAPSULE_THROW_HOOK hook)
{
    capsuleThrowHook = hook;
}

// Starts a standalone throw-path search between positions supplied by a board event.
// Arc height grows with distance and is capped at 700 world units.
void mbCapEffThrowCreate(HuVecF *startPos, HuVecF *endPos)
{
    HuVecF delta;
    float x[3];
    float y[3];
    float z[3];
    float dist;
    float yOfs;

    x[0] = startPos->x;
    x[1] = startPos->x;
    x[2] = endPos->x;
    y[0] = startPos->y;
    y[1] = startPos->y;
    y[2] = endPos->y;
    z[0] = startPos->z;
    z[1] = startPos->z;
    z[2] = endPos->z;
    PSVECSubtract(endPos, startPos, &delta);
    dist = PSVECMag(&delta);
    yOfs = 50.0f + (0.1f * dist);
    if (yOfs > 700.0f) {
        yOfs = 700.0f;
    }
    CapEffThrowCreate(-1, x, y, z, yOfs, -1);
}

// Scripted callers poll for the path bend and flight duration; success destroys the search.
BOOL mbCapEffThrowCheck(HuVecF *pos, int *maxTime)
{
    CAP_EFF_THROW_WORK *work;
    HU3D_MODEL *modelP;
    BOOL paramSetF;
    BOOL result;

    if (capEffThrowMdlId == MB_MODEL_NONE) {
        paramSetF = FALSE;
    } else {
        modelP = &Hu3DData[capEffThrowMdlId];
        work = modelP->hookData;
        if (!work->endF) {
            paramSetF = FALSE;
        } else {
            pos->x = work->x[1];
            pos->y = work->y[1];
            pos->z = work->z[1];
            *maxTime = work->maxTime;
            paramSetF = TRUE;
        }
    }
    result = paramSetF;
    if (result) {
        if (capEffThrowMdlId != MB_MODEL_NONE) {
            Hu3DModelKill(capEffThrowMdlId);
        }
        capEffThrowMdlId = MB_MODEL_NONE;
    }
    return result;
}

// Last-five-turn and capsule events call this to play a supplied arc without a player pose.
// startT is the initial flight fraction; values at least one go straight to the landing.
void mbCapAutoThrow(HuVecF *startPos, HuVecF *endPos, HuVecF *masuPos, int playerNo,
    int masuId, int capsuleNo, BOOL maxTime, float startT)
{
    CAP_AUTO_THROW_WORK *workData;
    CAP_AUTO_THROW_WORK *work;

    workData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*workData), HU_MEMNUM_OVL);
    work = workData;
    memset(work, 0, sizeof(*work));
    work->playerNo = playerNo;
    work->capsuleNo = capsuleNo;
    work->masuId = masuId;
    work->maxTime = maxTime;
    work->startT = startT;
    work->startPos = *startPos;
    work->endPos = *endPos;
    work->masuPos = *masuPos;
    CapAutoThrow(work);
    mbCapAutoThrowEnd(work);
    HuMemDirectFree(work);
}

// Runs the automatic throw's supplied arc, landing effects, and placement synchronously.
// These event throws request no bonus coins and bracket playback with the throw hook.
static void CapAutoThrow(CAP_AUTO_THROW_WORK *work)
{
    int capsuleNo;
    int masuId;
    int time;
    int seNo;
    int objColorId;
    float t;
    float glowScale;
    GXColor color;
    GXColor glowColor;
    HuVecF effPos;
    HuVecF particlePos;
    HuVecF particleVel;
    float x[3];
    float y[3];
    float z[3];
    HuVecF throwOut;
    HuVecF glowPos;
    HuVecF glowVel;
    HuVecF rot;
    HuVecF pos;
    GXColor *glowColorP;
    HuVecF *glowVelP;
    HuVecF *glowPosP;

    if (capsuleThrowHook) {
        capsuleThrowHook(TRUE);
    }
    capsuleNo = work->capsuleNo;
    masuId = work->masuId;
    if (masuId > 0) {
        mbMasuPosGet((s16)masuId, &pos);
        mbMasuRotGet((s16)masuId, &rot);
    }
    capsuleThrowGlowOMObj = mbev_CapEffGlowCreate();
    capsuleThrowRingOMObj = mbev_CapEffRingCreate();
    capsuleThrowRayOMObj = mbev_CapEffRayCreate(0.0f, 0.02f);
    if (masuId > 0) {
        mbev_CapEffRayTransformSet(capsuleThrowRayOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuHitOMObj = mbev_CapEffMasuHitCreate();
    if (masuId > 0) {
        mbev_CapEffMasuHitTransformSet(
            capsuleThrowMasuHitOMObj, &pos, &rot, NULL);
    }
    capsuleThrowMasuCoinOMObj = mbev_CapEffCoinCreate();
    CapEffTrailCreate(capsuleNo);
    CapEffCrackCreate();
    x[0] = work->startPos.x;
    x[1] = work->endPos.x;
    x[2] = work->masuPos.x;
    y[0] = work->startPos.y;
    y[1] = work->endPos.y;
    y[2] = work->masuPos.y;
    z[0] = work->startPos.z;
    z[1] = work->endPos.z;
    z[2] = work->masuPos.z;
    objColorId = mbCapObjColorCreate(work->capsuleNo, FALSE);
    mbCapObjColorPosSetV(objColorId, &work->startPos);
    mbCapObjColorScaleSet(objColorId, 1.0f, 1.0f, 1.0f);
    mbCapObjColorLayerSet(objColorId, 4);
    if (work->startT < 1.0f) {
        mbCameraMoveMasu((s16)work->masuId, NULL, NULL, 1600.0f, -1.0f, -1);
        mbCameraMoveWait();
        mbWipeSpecialFadeOutCreate(1, 60);
    }
    if (work->startT < 1.0f) {
        CapEffTrailAdd(&work->startPos, work->capsuleNo);
    }
    if (work->startT < 1.0f) {
        seNo = mbAudFXPlay(MSM_SE_BOARD_26);
    } else {
        seNo = -1;
    }
    time = work->startT * work->maxTime;
    do {
        if (work->startT >= 1.0f) {
            break;
        }
        time++;
        t = (float)time / (float)work->maxTime;
        CapThrowCameraCalc(t, x, y, z, &throwOut, 3);
        effPos.x = throwOut.x;
        effPos.y = throwOut.y;
        effPos.z = throwOut.z;
        mbCapObjColorPosSet(objColorId, effPos.x, effPos.y, effPos.z);
        mbCapObjColorScaleSet(objColorId, 0.5f, 0.5f, 0.5f);
        if (work->startT < 1.0f) {
            CapEffTrailPosSet(&effPos);
        }
        if (capsuleThrowGlowOMObj != NULL) {
            particlePos.x = effPos.x
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particlePos.y = effPos.y
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particlePos.z = effPos.z
                + ((0.5f - MBCapsuleEffRandF()) * 100.0f * 0.75f);
            particleVel.x = particleVel.y = particleVel.z = 0.0f;
            color = capsuleAutoThrowColorTbl[mbCapColorGet(work->capsuleNo)];
            color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));
            glowColor = color;
            glowColorP = &glowColor;
            glowVel = particleVel;
            glowVelP = &glowVel;
            glowPos = particlePos;
            glowPosP = &glowPos;
            glowScale = 100.0f
                * (0.2f + (0.025f * MBCapsuleEffRandF()));
            mbev_CapEffGlowAdd(capsuleThrowGlowOMObj, glowPosP, glowVelP,
                (int)(60.0f * (1.0f + MBCapsuleEffRandF())), glowScale,
                0.0f, 0.025f, glowColorP);
        }
        HuPrcVSleep();
    } while (time < work->maxTime);
    if (seNo != -1) {
        mbAudFXStop(seNo);
    }
    seNo = -1;
    mbAudFXPlay(MSM_SE_BOARD_27);
    effPos = work->endPos;
    effPos.y -= 10000.0f;
    CapEffTrailPosSet(&effPos);
    mbCapObjColorKill(objColorId);
    if (work->startT < 1.0f) {
        omVibrate(work->playerNo, 20, 7, 3);
    }
    CapEffThrowMasu(work->masuId, work->capsuleNo, work->playerNo, FALSE);
    HuPrcSleep(18);
    while (mbev_CapEffCoinNumGet(capsuleThrowMasuCoinOMObj) > 0) {
        HuPrcVSleep();
    }
    while (mbev_CapEffGlowDispGet(capsuleThrowGlowOMObj) != 0) {
        HuPrcVSleep();
    }
    if (capsuleThrowGlowOMObj != NULL) {
        mbev_CapEffGlowKill(capsuleThrowGlowOMObj);
    }
    if (capsuleThrowRingOMObj != NULL) {
        mbev_CapEffRingKill(capsuleThrowRingOMObj);
    }
    if (capsuleThrowRayOMObj != NULL) {
        mbev_CapEffRayKill(capsuleThrowRayOMObj);
    }
    if (capsuleThrowMasuHitOMObj != NULL) {
        mbev_CapEffMasuHitKill(capsuleThrowMasuHitOMObj);
    }
    if (capsuleThrowMasuCoinOMObj != NULL) {
        mbev_CapEffCoinKill(capsuleThrowMasuCoinOMObj);
    }
    CapEffCrackKill();
    CapEffTrailKill();
    if (work->masuId > 0) {
        CapThrowEndWin(work->masuId, work->capsuleNo);
    }
    if (work->startT < 1.0f) {
        mbWipeSpecialFadeInCreate(1, 1);
    }
    if (capsuleThrowHook) {
        capsuleThrowHook(FALSE);
    }
}

void mbCapAutoThrowEnd(CAP_AUTO_THROW_WORK *work)
{
}

// Called by mbCapUse to choose a capsule destination or route to use/discard prompts.
// Returns a space ID, -1 for cancel, -2 for discard, or -3 for use on the player.
int mbCapSelectMasu(int playerNo, int capsuleNo)
{
    CAP_SELECT_MASU_WORK *workData;
    CAP_SELECT_MASU_WORK *work;

    capsuleNo = (u8)capsuleNo;
    switch (mbCapUseModeGet((s16)capsuleNo)) {
        case 0:
            capsuleMasuSelectResult = -3;
            return capsuleMasuSelectResult;
        case 3:
        case 4:
            capsuleMasuSelectResult = -2;
            return capsuleMasuSelectResult;
    }

    capsuleMasuSelectResult = -1;
    capsuleMasuSelectComF[playerNo] = FALSE;
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, 32, HU_MEMNUM_OVL);
    work = workData;
    work->playerNo = playerNo;
    work->unusedResult = -1;
    work->capsuleNo = capsuleNo;
    if (!GwPlayer[playerNo].comF) {
        CapSelectMasuPlayer(work);
    } else {
        CapSelectMasuCom(work);
    }
    HuMemDirectFree(work);
    return capsuleMasuSelectResult;
}

// Scans reachable placement counts from every space during capsule initialization.
// The computed maximum is not retained after the scan.
void mbCapSelectMasuInit(void)
{
    int maxNum = 0;
    s16 *masuFlagData;
    s16 *masuFlag;
    int masuId;
    int i;
    int num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    for (masuId = 1; masuId <= mbMasuRawNumGet(); masuId++) {
        memset(masuFlag, 0, sizeof(s16) * 256);
        capsuleMasuSelectEndF = TRUE;
        CapSelectMasuListGet(masuFlag, masuId, 5, 5);
        for (i = 0, num = 1; i < 256; i++) {
            if (masuFlag[i] & 1) {
                num++;
            }
        }
        if (num > maxNum) {
            maxNum = num;
        }
    }
    HuMemDirectFree(masuFlag);
}

// Runs human-controlled destination selection for mbCapSelectMasu.
// Moves the camera along eligible routes and restores a saved preview on cancellation.
static int CapSelectMasuPlayer(CAP_SELECT_MASU_WORK *work)
{
    s16 candidateDir[10];
    s16 candidates[10];
    s16 links[10];
    float candidateRot[10];
    float candidateAngle[10];
    s16 *masuFlag;
    HuVecF playerPos;
    HuVecF moveTargetPos;
    HuVecF moveDelta;
    HuVecF targetPos;
    HuVecF cameraOffset;
    HuVecF rot;
    HuVecF stickDir;
    HuVecF delta;
    HuVecF currentPos;
    HuVec2f winPos;
    int padNo;
    int masuId;
    int linkNum;
    int frontNum;
    int validParentNum;
    int validF;
    int capsuleMes;
    int directF;
    s16 direction;
    int i;
    int time;
    int previousMasuId;
    int maxTime;
    int hiliteDelay;
    int capsuleMesPrev;
    int initialF;
    GXColor color;
    int moveNum;
    int selectedId;
    int oldObjId;
    int oldResult;
    float t;
    float scale;
    float diffAngle;
    float bestAngle;
    float cameraZoom;
    float angleEnd;
    float angle;
    moveNum = -1;
    memset(candidateDir, 0, sizeof(candidateDir));
    memset(candidates, 0, sizeof(candidates));
    padNo = GwPlayer[work->playerNo].padNo;
    masuId = previousMasuId = GwPlayer[work->playerNo].masuId;
    frontNum = 0;
    validParentNum = 0;
    direction = 1;
    hiliteDelay = 0;
    masuFlag = (CapEffHiliteCreate(), HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL));
    memset(masuFlag, 0, sizeof(s16) * 256);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuListGet(masuFlag, masuId, 5, 5);
    masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;

    mbCapSelectResultGet(work->playerNo, &oldObjId, &oldResult);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        work->capObjId = capsuleObjId = oldObjId;
    } else {
        work->capObjId = capsuleObjId = mbCapObjCreate(work->capsuleNo, TRUE);
        mbObjDispSet(work->capObjId, FALSE);
        mbPlayerPosGet(work->playerNo, &playerPos);
        mbObjPosSet(work->capObjId, playerPos.x, playerPos.y + 250.0f,
            playerPos.z);
        mbObjScaleSet(work->capObjId, 1.2f, 1.2f, 1.2f);
    }
    mbObjLayerSet(work->capObjId, 4);
    {
        s16 objId;

        objId = work->capObjId;
        mbObjAttrSet(objId, HU3D_MOTATTR_LOOP);
    }
    cameraZoom = mbCameraZoomGet();
    mbCameraOffsetGet(&cameraOffset);

    work->cameraObjId = mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, TRUE);
    mbObjDispSet(work->cameraObjId, FALSE);
    mbPlayerPosGet(work->playerNo, &playerPos);
    mbObjPosSetV(work->cameraObjId, &playerPos);
    mbCameraMoveObj(work->cameraObjId, NULL, NULL, 3200.0f, -1.0f, 15);
    initialF = TRUE;
    work->helpWinId = mbWinCreateHelp(MESSNUM(
        MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_SELECT_HELP));
    mbWinTopPosGet(&winPos);
    mbWinTopPosSet(winPos.x, 284);
    mbWinAttrSet(work->helpWinId, 2048);
    work->messageWinId = MB_MODEL_NONE;
    capsuleMes = capsuleMesPrev = -1;
    CapGuideCreate();
    CapEffMasuOkCreate();
    CapEffMasuOkAddAll((s16)masuId, masuFlag);
    CapEffMasuOkDispSet(TRUE);

    for (;;) {
        mbMasuPosGet((s16)masuId, &currentPos);
        if (masuFlag[masuId] & 1) {
            validF = TRUE;
            CapEffMasuOkPosSet(&currentPos, masuId);
        } else {
            validF = FALSE;
        }

        linkNum = mbMasuLinkTblGet((s16)masuId, links);
        for (i = 0, frontNum = 0; i < linkNum; i++) {
            if (masuFlag[links[i]] != 0) {
                candidates[frontNum] = links[i];
                frontNum++;
            }
        }
        linkNum = frontNum;
        linkNum += mbMasuLinkParentGet(
            (s16)masuId, &candidates[linkNum]);
        for (i = 0, validParentNum = 0; i < linkNum; i++) {
            if (i < frontNum) {
                continue;
            }
            if (masuFlag[candidates[i]] == 0) {
                candidates[i] = -1;
            } else {
                validParentNum++;
            }
        }
        if (!validF && frontNum == 1 && validParentNum == 1 &&
            masuId != GwPlayer[work->playerNo].masuId) {
            directF = TRUE;
        } else {
            directF = FALSE;
        }

        if (!directF) {
            // Both positions deliberately use the same destination prompt.
            if (masuId == GwPlayer[work->playerNo].masuId) {
                capsuleMes = 0;
            } else {
                capsuleMes = 0;
            }
            if (capsuleMes != capsuleMesPrev) {
                if (work->messageWinId != MB_MODEL_NONE) {
                    mbWinKill(work->messageWinId);
                }
                work->messageWinId = MB_MODEL_NONE;
                work->messageWinId = CapSelectMasuWinCreate(capsuleMes);
                capsuleMesPrev = capsuleMes;
            }
        }

        for (i = 0; i < linkNum; i++) {
            s16 heading;

            if (candidates[i] == -1) {
                continue;
            }
            mbMasuPosGet(candidates[i], &targetPos);
            PSVECSubtract(&targetPos, &currentPos, &delta);
            angleEnd = (float)(90.0 -
                ((atan2(delta.z, delta.x) / M_PI) * 180.0));
            OSf32tos16(&angleEnd, &heading);
            if (heading < 0) {
                heading += 360;
            }
            if (heading > 360) {
                heading -= 360;
            }
            candidateRot[i] = (float)heading;
            candidateAngle[i] = CapCameraXZAngleGet((float)heading);
            heading = (s16)(((heading + 22) / 45) * 45);
            candidateDir[i] = heading;
        }
        if (!directF) {
            for (i = 0; i < linkNum; i++) {
                if (candidates[i] != -1 && masuFlag[candidates[i]] != 0) {
                    CapGuideRotYSet(masuId, candidateRot[i]);
                }
            }
        }
        if (validF) {
            mbAudFXPlay(MSM_SE_BRD00_23);
        }
        do {
        stickDir.x = (float)mbPadStkXGet(padNo);
        stickDir.y = 0.0f;
        stickDir.z = (float)mbPadStkYGet(padNo);

        if (initialF) {
            mbObjDispSet(work->capObjId, TRUE);
            time = 0;
            do {
                if (oldObjId == MB_MODEL_NONE) {
                    time++;
                    t = (float)time / 18.0f;
                    if (t > 1.0f) {
                        t = 1.0f;
                    }
                    mbPlayerPosGet(work->playerNo, &playerPos);
                    playerPos.y += 100.0 +
                        (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
                    mbObjPosSetV(work->capObjId, &playerPos);
                    scale = (double)1.2f *
                        sin((M_PI * (90.0f * t)) / 180.0);
                    mbObjScaleSet(work->capObjId, scale, scale, scale);
                } else {
                    t = 1.0f;
                }
                HuPrcVSleep();
            } while (!mbCameraMoveCheck() || t < 1.0f);
            initialF = FALSE;
        }

        if (GwPlayer[work->playerNo].comF) {
            capsuleMasuSelectResult = -1;
            goto cleanup;
        }

        if (validF) {
            if (--hiliteDelay <= 0) {
                mbMasuPosGet((s16)masuId, &targetPos);
                mbMasuRotGet((s16)masuId, &rot);
                targetPos.y += 3.0f;
                rot.x += capsuleMasuSelectRotTbl[0].x;
                rot.y += capsuleMasuSelectRotTbl[0].y;
                rot.z += capsuleMasuSelectRotTbl[0].z;
                color = capsuleMasuSelectColorTbl[mbCapColorGet(work->capsuleNo)];
                CapEffHiliteAdd(targetPos, rot, capsuleMasuSelectRotTbl[1],
                    1, 18, 1, 2, color);
                hiliteDelay = 19;
            }
        }

        if (directF) {
            if (direction > 0) {
                for (i = 0; i < frontNum; i++) {
                    if (candidates[i] != -1) {
                        masuId = candidates[i];
                        break;
                    }
                }
                moveNum = 0;
            } else {
                for (i = frontNum; i < linkNum; i++) {
                    if (candidates[i] != -1) {
                        masuId = candidates[i];
                        break;
                    }
                }
            }
        } else {
            if (HuPadBtnDown[padNo] & PAD_BUTTON_A) {
                if (validF) {
                    mbAudFXPlay(MSM_SE_BRD00_24);
                    capsuleMasuSelectResult = masuId;
                    goto cleanup;
                }
            } else if (HuPadBtnDown[padNo] & PAD_BUTTON_B) {
                capsuleMasuSelectResult = -1;
                goto cleanup;
            } else if (fabs(stickDir.x) >= 8.0 ||
                fabs(stickDir.z) >= 8.0) {
                angle = (float)((atan2(mbPadStkXGet(padNo),
                    -mbPadStkYGet(padNo)) / M_PI) * 180.0);
                diffAngle = 180.0f;
                bestAngle = 180.0f;
                selectedId = -1;
                // The angle threshold stays fixed, so the last qualifying route wins.
                for (i = 0; i < linkNum; i++) {
                    if (candidates[i] == -1 ||
                        masuFlag[candidates[i]] == 0) {
                        continue;
                    }
                    diffAngle = CapAngleSumWrap(candidateAngle[i], angle);
                    if (fabs(diffAngle) < 45.0 &&
                        fabs(diffAngle) < bestAngle) {
                        selectedId = candidates[i];
                    }
                }
                if (selectedId != -1) {
                    masuId = selectedId;
                    moveNum = 0;
                }
            }
        }
        HuPrcVSleep();
        } while (masuId == previousMasuId);
        CapGuideGrowSet();
        CapEffMasuOkNext();
        playerPos = currentPos;
        mbMasuPosGet((s16)masuId, &moveTargetPos);
        PSVECSubtract(&moveTargetPos, &playerPos, &moveDelta);
        t = PSVECMag(&moveDelta);
        maxTime = (int)(t / 30.000002f);
        for (time = 1; time < maxTime; time++) {
            t = (float)time / (float)maxTime;
            PSVECSubtract(&moveTargetPos, &playerPos, &moveDelta);
            PSVECScale(&moveDelta, &moveDelta, t);
            PSVECAdd(&playerPos, &moveDelta, &moveDelta);
            mbObjPosSet(work->cameraObjId, moveDelta.x, moveDelta.y, moveDelta.z);
            HuPrcVSleep();
        }
        for (time = 0; time < frontNum; time++) {
            if (masuId == candidates[time]) {
                break;
            }
        }
        if (time < frontNum) {
            direction = 1;
        } else {
            direction = -1;
        }
        previousMasuId = masuId;
    }

cleanup:
    HuMemDirectFree(masuFlag);
    if (work->helpWinId != MB_MODEL_NONE) {
        mbWinKill(work->helpWinId);
    }
    work->helpWinId = MB_MODEL_NONE;
    if (work->messageWinId != MB_MODEL_NONE) {
        mbWinKill(work->messageWinId);
    }
    work->messageWinId = MB_MODEL_NONE;
    if (capsuleMasuSelectResult != -1) {
        mbWipeDissolveFadeOut();
    } else {
        if (oldObjId != MB_MODEL_NONE) {
            mbCapSelectResultSet(work->playerNo, oldObjId, oldResult);
            work->capObjId = MB_MODEL_NONE;
        }
        if (work->capObjId != MB_MODEL_NONE) {
            mbCapObjKill(work->capObjId);
        }
        work->capObjId = MB_MODEL_NONE;
        capsuleObjId = MB_MODEL_NONE;
    }
    mbCameraMovePlayer(
        (s16)work->playerNo, NULL, &cameraOffset, cameraZoom, -1.0f, -1);
    mbCameraMoveWait();
    CapSelectMasuKill(work);
}

// Chooses a random eligible destination for mbCapSelectMasu's computer-player path.
// Prefers spaces without capsules; tutorial guidance can replace the chosen space.
static int CapSelectMasuCom(CAP_SELECT_MASU_WORK *work)
{
    s16 links[10];
    s16 parents[10];
    s16 count;
    s16 i;
    int masuId;
    s16 *masuFlag;
    s16 targetId;
    s16 *path;
    s16 *masuList;
    int time;
    int capsuleMes;
    HuVecF playerPos;
    HuVecF cameraOffset;
    int previousMasuId;
    int linkCount;
    int validLinkNum;
    float t;
    float scale;
    float cameraZoom;
    int moveNum;
    GW_PLAYER *volatile playerP;
    int maxMasu;
    int padNo;
    int oldObjId;
    int oldResult;
    int selPos;
    int j;
    MBCAMERA *cameraP;
    BOOL sePlayF;
    BOOL modelDispF;
    int playerNo;
    GW_PLAYER *playerData;
    GW_PLAYER *playerBase;
    s16 *masuFlagData;
    s16 *masuListData;
    s16 *pathData;
    s16 dir;

    moveNum = -1;
    memset(links, 0, sizeof(links));
    memset(parents, 0, sizeof(parents));
    (void)time;
    (void)time;
    (void)j;
    (void)j;
    (void)cameraP;
    (void)cameraP;
    {
        playerNo = work->playerNo;
        playerData = &GwPlayer[playerNo];
        playerBase = playerData;
        playerP = playerBase;

        maxMasu = mbMasuRawNumGet();
        padNo = GwPlayer[work->playerNo].padNo;
        masuId = previousMasuId = playerP->masuId;
    }
    linkCount = 0;
    validLinkNum = 0;
    dir = 1;
    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    memset(masuFlag, 0, sizeof(s16) * 256);
    masuListData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuList = masuListData;
    memset(masuList, 0, sizeof(s16) * 256);
    pathData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    path = pathData;
    memset(path, 0, sizeof(s16) * 256);

    if (capsuleSelectComBack) {
        capsuleMasuSelectEndF = TRUE;
        CapSelectMasuAddBack(masuFlag, masuId, 5);
        if (masuId != mbMasuFind_AttrIdGet(-1, MASU_FLAG_START)) {
            CapSelectMasuLinkCheck(masuFlag,
                mbMasuTypeFindLink(
                    mbMasuFind_AttrIdGet(-1, MASU_FLAG_START), 0));
        }
    } else {
        capsuleMasuSelectEndF = TRUE;
        CapSelectMasuListGet(masuFlag, masuId, 5, 5);
    }
    masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;

    for (i = 1, count = 0; i < 256; i++) {
        if (i != masuId && (masuFlag[i] & 1) &&
            mbMasuCapsuleGet(i) == -1) {
            masuList[count] = i;
            count++;
        }
    }
    if (count <= 0) {
        for (i = 1, count = 0; i < 256; i++) {
            if (i != masuId && (masuFlag[i] & 1)) {
                masuList[count] = i;
                count++;
            }
        }
    }
    if (count <= 0) {
        targetId = -1;
    } else if (count <= 1) {
        targetId = masuList[0];
    } else {
        targetId = masuList[mbRandMod((s16)count)];
    }

    (void)masuId;
    (void)masuId;
    (void)masuId;
    (void)masuId;
    (void)targetId;

    mbCapSelectResultGet(work->playerNo, &oldObjId, &oldResult);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        work->capObjId = capsuleObjId = oldObjId;
    } else {
        work->capObjId = capsuleObjId = mbCapObjCreate(work->capsuleNo, TRUE);
        mbObjDispSet(work->capObjId, TRUE);
        mbPlayerPosGet(work->playerNo, &playerPos);
        mbObjPosSet(work->capObjId, playerPos.x, playerPos.y + 250.0f,
            playerPos.z);
        mbObjScaleSet(work->capObjId, 1.2f, 1.2f, 1.2f);
        (void)scale;
        (void)scale;
    }
    mbObjLayerSet(work->capObjId, 4);
    {
        s16 objId;

        objId = work->capObjId;
        mbObjAttrSet(objId, HU3D_MOTATTR_LOOP);
    }
    cameraZoom = mbCameraZoomGet();
    (void)t;
    (void)t;
    mbCameraOffsetGet(&cameraOffset);

    work->cameraObjId = mbObjCreate(mbCapFileGet(work->capsuleNo), NULL, TRUE);
    mbObjDispSet(work->cameraObjId, FALSE);
    mbPlayerPosGet(work->playerNo, &playerPos);
    mbObjPosSetV(work->cameraObjId, &playerPos);
    modelDispF = TRUE;
    (void)modelDispF;
    work->helpWinId = MB_MODEL_NONE;
    work->messageWinId = MB_MODEL_NONE;
    sePlayF = -1;
    selPos = sePlayF;
    (void)selPos;
    CapGuideCreate();
    CapEffMasuOkCreate();

    capsuleComSearchF = FALSE;
    CapSelectMasuComListGet(path, masuFlag, masuId, targetId, 0);
    if (!capsuleComSearchF) {
        capsuleComSearchF = FALSE;
        CapSelectMasuComListGetRev(path, masuFlag, targetId, masuId, 0);
    }
    for (count = 0; count < 256; count++) {
        if (targetId == path[count]) {
            break;
        }
    }
    // A failed path search still appends the chosen target here.
    if (count < 256 || !capsuleComSearchF) {
        path[count + 1] = targetId;
        path[count + 2] = targetId;
        count += 2;
    } else {
        capsuleMasuSelectComF[work->playerNo] = TRUE;
        capsuleMasuSelectResult = masuId = -1;
        goto cleanup;
    }

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        targetId = mbTutorialCall(17);
    }
    capsuleMes = -1;
    (void)capsuleMes;
    mbAudFXPlay(MSM_SE_BRD00_24);
    capsuleMasuSelectResult = (s16)targetId;

cleanup:
    if (work->helpWinId != MB_MODEL_NONE) {
        mbWinKill(work->helpWinId);
    }
    work->helpWinId = MB_MODEL_NONE;
    if (work->messageWinId != MB_MODEL_NONE) {
        mbWinKill(work->messageWinId);
    }
    work->messageWinId = MB_MODEL_NONE;
    HuMemDirectFree(masuFlag);
    HuMemDirectFree(path);
    HuMemDirectFree(masuList);
    if (capsuleMasuSelectResult == -1) {
        if (oldObjId != MB_MODEL_NONE) {
            mbCapSelectResultSet(work->playerNo, oldObjId, oldResult);
            work->capObjId = MB_MODEL_NONE;
        }
        if (work->capObjId != MB_MODEL_NONE) {
            mbCapObjKill(work->capObjId);
        }
        work->capObjId = MB_MODEL_NONE;
        capsuleObjId = MB_MODEL_NONE;
    }
    CapSelectMasuKill(work);
}

// Releases route guides, space markers, camera focus model, and selection windows.
// Both destination selection paths call this after preserving any capsule preview.
static void CapSelectMasuKill(CAP_SELECT_MASU_WORK *work)
{
    CapGuideKill();
    CapEffMasuOkKill();
    if (work->cameraObjId != MB_MODEL_NONE) {
        mbObjKill(work->cameraObjId);
    }
    work->cameraObjId = MB_MODEL_NONE;
    if (work->helpWinId != MB_MODEL_NONE) {
        mbWinKill(work->helpWinId);
    }
    work->helpWinId = MB_MODEL_NONE;
    if (work->messageWinId != MB_MODEL_NONE) {
        mbWinKill(work->messageWinId);
    }
    work->messageWinId = MB_MODEL_NONE;
    CapEffHiliteKill();
}

// Candidate enumeration accepts regular spaces with compatible capsules and no player on them.
static BOOL CapSelectMasuCheck(int masuId)
{
    int masuType;
    u32 masuMAttr;
    int i;

    masuType = mbMasuTypeGet(masuId);
    masuMAttr = mbMasuMAttrGet(masuId);
    if (masuType == 1 || masuType == 2) {
        if (mbCapMasuDispTypeGet(masuId) == 0 ||
            mbCapMasuDispTypeGet(masuId) == 1) {
            for (i = 0; i < GW_PLAYER_MAX; i++) {
                if (masuId == GwPlayer[i].masuId) {
                    return FALSE;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL CapSelectMasuDispCheck(int masuId)
{
    return mbMasuDispCheck(masuId);
}

// Builds the forward/backward destination table for selection and range-count queries.
// Trims marked connector paths near the board start after both traversals.
static void CapSelectMasuListGet(
    s16 *masuFlag, s16 masuId, s16 frontMax, s16 backMax)
{
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddFront(masuFlag, masuId, frontMax);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddBack(masuFlag, masuId, backMax);
    if (masuId != mbMasuFind_AttrIdGet(-1, MASU_FLAG_START)) {
        CapSelectMasuLinkCheck(masuFlag,
            mbMasuTypeFindLink(
                mbMasuFind_AttrIdGet(-1, MASU_FLAG_START), 0));
    }
}

// Walks outgoing links while building the capsule destination table.
// Displayed spaces after the starting space consume the remaining forward range.
static void CapSelectMasuAddFront(s16 *masuFlag, s16 masuId, s16 max)
{
    s16 link[10];
    s16 linkNum;
    int i;

    linkNum = mbMasuLinkTblGet(masuId, link);
    if (CapSelectMasuCheck(masuId)) {
        masuFlag[masuId] = 1;
    } else {
        masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;
    }
    if (CapSelectMasuDispCheck(masuId) && !capsuleMasuSelectEndF) {
        max--;
    }
    capsuleMasuSelectEndF = FALSE;
    if (max > 0) {
        for (i = 0; i < linkNum; i++) {
            if (mbev_CapMasuMoveCheck(link[i])) {
                continue;
            }
            if (i == 0 && (mbMasuAttrGet(masuId) & MASU_FLAG_BATTAN)) {
                continue;
            }
            CapSelectMasuAddFront(masuFlag, link[i], max);
        }
    }
}

// Walks incoming links while building the capsule destination table.
// Displayed spaces after the starting space consume the remaining backward range.
static void CapSelectMasuAddBack(s16 *masuFlag, s16 masuId, s16 max)
{
    s16 parent[10];
    s16 parentNum;
    s16 endF;
    int i;
    int num;

    if (CapSelectMasuCheck(masuId)) {
        masuFlag[masuId] = 1;
    } else {
        masuFlag[masuId] = CAPSULE_MASU_SELECT_BLOCKED;
    }
    if (mbMasuDispCheck(masuId) && mbev_CapMasuMoveCheck(masuId)) {
        max = 0;
    }
    if (CapSelectMasuDispCheck(masuId) && !capsuleMasuSelectEndF) {
        max--;
    }
    endF = capsuleMasuSelectEndF;
    capsuleMasuSelectEndF = FALSE;
    num = 0;
    if (max > 0) {
        parentNum = mbMasuLinkParentGet(masuId, parent);
        for (i = 0; i < parentNum; i++) {
            if (!mbev_CapMasuMoveCheck(parent[i]) || mbMasuDispCheck(parent[i])) {
                CapSelectMasuAddBack(masuFlag, parent[i], max);
                num++;
            }
        }
    }
    if (!endF && num <= 0 && mbMasuTypeGet(masuId) == 0) {
        CapSelectMasuLinkCheck(masuFlag, masuId);
    }
}

// Trims marked type-zero connector routes during destination enumeration.
// Non-connector spaces keep their destination-table entry.
static void CapSelectMasuLinkCheck(s16 *masuFlag, s16 masuId)
{
    s16 link[10];
    s16 linkNum;
    int i;

    if (masuId >= 1) {
        linkNum = mbMasuLinkTblGet(masuId, link);
        if (masuFlag[masuId] != 0) {
            if (mbMasuTypeGet(masuId) == 0) {
                masuFlag[masuId] = 0;
            } else {
                return;
            }
            for (i = 0; i < linkNum; i++) {
                if (!mbev_CapMasuMoveCheck(link[i])) {
                    CapSelectMasuLinkCheck(masuFlag, link[i]);
                }
            }
        }
    }
}

// Searches marked outgoing links for the computer player's destination and records the path.
// Called by computer selection; capsuleComSearchF stops traversal once the target is found.
static int CapSelectMasuComListGet(
    s16 *path, s16 *masuFlag, s16 masuId, s16 targetId, int depth)
{
    s16 links[10];
    s16 childLinks[10];
    s16 grandchildLinks[10];
    s16 branchLinks[10];
    s16 leafLinks[10];
    s16 linkNum;
    s16 childLinkNum;
    s16 grandchildLinkNum;
    s16 branchLinkNum;
    s16 childMasuId;
    s16 grandchildMasuId;
    s16 branchMasuId;
    s16 leafLinkNum;
    s16 leafMasuId;
    s16 unusedLeafMasuId;
    int i;
    int j;
    int k;
    int l;
    int m;
    int childDepth;
    int grandchildDepth;
    int branchDepth;
    int leafDepth;

    path[(s16)depth] = masuId;
    depth++;
    if (masuId == targetId) {
        capsuleComSearchF = TRUE;
        return (s16)depth;
    }

    linkNum = mbMasuLinkTblGet(masuId, links);
    for (i = 0; i < linkNum; i++) {
        if (masuFlag[links[i]] != 0) {
            childDepth = depth;
            childMasuId = links[i];
            path[(s16)childDepth] = childMasuId;
            childDepth++;
            if (childMasuId == targetId) {
                capsuleComSearchF = TRUE;
            } else {
                childLinkNum = mbMasuLinkTblGet(childMasuId, childLinks);
                for (j = 0; j < childLinkNum; j++) {
                    if (masuFlag[childLinks[j]] != 0) {
                        grandchildDepth = childDepth;
                        grandchildMasuId = childLinks[j];
                        path[(s16)grandchildDepth] = grandchildMasuId;
                        grandchildDepth++;
                        if (grandchildMasuId == targetId) {
                            capsuleComSearchF = TRUE;
                        } else {
                            grandchildLinkNum = mbMasuLinkTblGet(grandchildMasuId, grandchildLinks);
                            for (k = 0; k < grandchildLinkNum; k++) {
                                if (masuFlag[grandchildLinks[k]] != 0) {
                                    branchDepth = grandchildDepth;
                                    branchMasuId = grandchildLinks[k];
                                    path[(s16)branchDepth] = branchMasuId;
                                    branchDepth++;
                                    if (branchMasuId == targetId) {
                                        capsuleComSearchF = TRUE;
                                    } else {
                                        branchLinkNum = mbMasuLinkTblGet(
                                            branchMasuId, branchLinks);
                                        for (l = 0; l < branchLinkNum; l++) {
                                            if (masuFlag[branchLinks[l]] != 0) {
                                                leafDepth = branchDepth;
                                                unusedLeafMasuId = leafMasuId = branchLinks[l];
                                                path[(s16)leafDepth] = leafMasuId;
                                                leafDepth++;
                                                if (leafMasuId == targetId) {
                                                    capsuleComSearchF = TRUE;
                                                } else {
                                                    leafLinkNum = mbMasuLinkTblGet(
                                                        leafMasuId, leafLinks);
                                                    for (m = 0;
                                                         m < leafLinkNum; m++) {
                                                        if (masuFlag[leafLinks[m]]
                                                            != 0) {
                                                            CapSelectMasuComListGet(
                                                                path, masuFlag,
                                                                leafLinks[m],
                                                                targetId,
                                                                leafDepth);
                                                            if (capsuleComSearchF) {
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                                if (capsuleComSearchF) {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (capsuleComSearchF) {
                                        break;
                                    }
                                }
                            }
                        }
                        if (capsuleComSearchF) {
                            break;
                        }
                    }
                }
            }
            if (capsuleComSearchF) {
                return (s16)depth;
            }
        }
    }
    return (s16)depth;
}

// Searches marked incoming links when the computer player's forward search fails.
// Records the reverse path and stops when capsuleComSearchF reports the target.
static int CapSelectMasuComListGetRev(
    s16 *path, s16 *masuFlag, s16 masuId, s16 targetId, int depth)
{
    s16 parents[10];
    s16 childParents[10];
    s16 grandchildParents[10];
    s16 branchParents[10];
    s16 leafParents[10];
    s16 parentNum;
    s16 childParentNum;
    s16 grandchildParentNum;
    s16 branchParentNum;
    s16 childMasuId;
    s16 grandchildMasuId;
    s16 branchMasuId;
    s16 leafParentNum;
    s16 leafMasuId;
    s16 unusedLeafMasuId;
    int i;
    int j;
    int k;
    int l;
    int m;
    int childDepth;
    int grandchildDepth;
    int branchDepth;
    int leafDepth;

    path[(s16)depth] = masuId;
    depth++;
    if (masuId == targetId) {
        capsuleComSearchF = TRUE;
        return (s16)depth;
    }

    parentNum = mbMasuLinkParentGet(masuId, parents);
    for (i = 0; i < parentNum; i++) {
        if (masuFlag[parents[i]] != 0) {
            childDepth = depth;
            childMasuId = parents[i];
            path[(s16)childDepth] = childMasuId;
            childDepth++;
            if (childMasuId == targetId) {
                capsuleComSearchF = TRUE;
            } else {
                childParentNum = mbMasuLinkParentGet(childMasuId, childParents);
                for (j = 0; j < childParentNum; j++) {
                    if (masuFlag[childParents[j]] != 0) {
                        grandchildDepth = childDepth;
                        grandchildMasuId = childParents[j];
                        path[(s16)grandchildDepth] = grandchildMasuId;
                        grandchildDepth++;
                        if (grandchildMasuId == targetId) {
                            capsuleComSearchF = TRUE;
                        } else {
                            grandchildParentNum =
                                mbMasuLinkParentGet(grandchildMasuId, grandchildParents);
                            for (k = 0; k < grandchildParentNum; k++) {
                                if (masuFlag[grandchildParents[k]] != 0) {
                                    branchDepth = grandchildDepth;
                                    branchMasuId = grandchildParents[k];
                                    path[(s16)branchDepth] = branchMasuId;
                                    branchDepth++;
                                    if (branchMasuId == targetId) {
                                        capsuleComSearchF = TRUE;
                                    } else {
                                        branchParentNum = mbMasuLinkParentGet(
                                            branchMasuId, branchParents);
                                        for (l = 0; l < branchParentNum; l++) {
                                            if (masuFlag[branchParents[l]] != 0) {
                                                leafDepth = branchDepth;
                                                unusedLeafMasuId = leafMasuId = branchParents[l];
                                                path[(s16)leafDepth] = leafMasuId;
                                                leafDepth++;
                                                if (leafMasuId == targetId) {
                                                    capsuleComSearchF = TRUE;
                                                } else {
                                                    leafParentNum = mbMasuLinkParentGet(
                                                        leafMasuId, leafParents);
                                                    for (m = 0;
                                                         m < leafParentNum; m++) {
                                                        if (masuFlag[leafParents[m]]
                                                            != 0) {
                                                            CapSelectMasuComListGetRev(
                                                                path, masuFlag,
                                                                leafParents[m],
                                                                targetId,
                                                                leafDepth);
                                                            if (capsuleComSearchF) {
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                                if (capsuleComSearchF) {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                    if (capsuleComSearchF) {
                                        break;
                                    }
                                }
                            }
                        }
                        if (capsuleComSearchF) {
                            break;
                        }
                    }
                }
            }
            if (capsuleComSearchF) {
                return (s16)depth;
            }
        }
    }
    return (s16)depth;
}

// Creates the destination prompt during human capsule selection and raises it eight pixels.
// Only message type zero is handled; other values leave the returned window ID unset.
static int CapSelectMasuWinCreate(int messageType)
{
    int winId;
    HuVec2f pos;

    switch (messageType) {
        case 0:
            winId = mbWinCreate(
                MBWIN_TYPE_CAPSULE,
                MESSNUM(MESS_CAPSULE_EX99,
                    CAPSULE_EX99_MESSAGE_SELECT_CAPSULE),
                -1);
            break;
    }
    mbWinMesSpeedSet(winId, 0);
    mbWinPause(winId);
    mbWinTopPosGet(&pos);
    mbWinTopPosSet(pos.x, pos.y - 8.0f);
    return winId;
}

// Called by mbCapUse for capsule modes that request a discard confirmation.
// Creates the prompt work and returns whether the player confirmed discarding.
static int CapUseDelete(int playerNo, int capsuleNo)
{
    CAP_USE_DELETE_WORK *work;
    int result;
    CAP_USE_DELETE_WORK *workData;

    capsuleNo = mbCapValueTypeGet((s16)capsuleNo);
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    work = workData;
    memset(work, 0, sizeof(*work));
    work->playerNo = playerNo;
    work->unusedModelId = MB_MODEL_NONE;
    work->capsuleNo = capsuleNo;
    capsuleMasuSelectComF[playerNo] = FALSE;
    result = CapUseDeleteWin(work);
    HuMemDirectFree(work);
    return result;
}

// Runs the discard confirmation, preview animation, and removal effect for CapUseDelete.
// Cancellation restores a saved preview; capsuleUseRemoveOnF suppresses inventory removal.
static int CapUseDeleteWin(CAP_USE_DELETE_WORK *work)
{
    HuVecF playerPos;
    HuVecF removePos;
    MBMODELID modelId;
    HuVecF *removePosP;
    int oldObjId;
    int result;
    int capsuleSlot;
    int choice;
    int time;
    float t;
    float scale;

    choice = -1;
    mbCapSelectResultGet(work->playerNo, &oldObjId, &result);
    mbCapSelectResultReset(work->playerNo);
    if (oldObjId != MB_MODEL_NONE) {
        work->capObjId = capsuleObjId = oldObjId;
    } else {
        work->capObjId = capsuleObjId = mbCapObjCreate(work->capsuleNo, TRUE);
        mbObjDispSet(work->capObjId, FALSE);
        mbPlayerPosGet(work->playerNo, &playerPos);
        mbObjPosSet(work->capObjId, playerPos.x, playerPos.y + 250.0f,
            playerPos.z);
        mbObjScaleSet(work->capObjId, 1.2f, 1.2f, 1.2f);
    }
    mbObjLayerSet(work->capObjId, 4);
    modelId = work->capObjId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    work->auxModelId = MB_MODEL_NONE;
    work->primaryWinId = MB_MODEL_NONE;
    work->secondaryWinId = MB_MODEL_NONE;
    mbObjDispSet(work->capObjId, TRUE);
    time = 0;
    do {
        if (oldObjId == MB_MODEL_NONE) {
            time++;
            t = (float)time / 18.0f;
            if (t > 1.0f) {
                t = 1.0f;
            }
            mbPlayerPosGet(work->playerNo, &playerPos);
            playerPos.y += 100.0 +
                (150.0 * sin((M_PI * (90.0f * t)) / 180.0));
            mbObjPosSetV(work->capObjId, &playerPos);
            scale = (double)1.2f *
                sin((M_PI * (90.0f * t)) / 180.0);
            mbObjScaleSet(work->capObjId, scale, scale, scale);
        } else {
            t = 1.0f;
        }
        HuPrcVSleep();
    } while (!mbCameraMoveCheck() || t < 1.0f);
    mbWinCreateChoice(
        MBWIN_TYPE_EVENT,
        MESSNUM(MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_DELETE_CHOICE),
        -1, TRUE);
    mbWinTopInsertMesSet(mbCapUseMesGet(work->capsuleNo), 0);
    if (GwPlayer[work->playerNo].comF) {
        CapComChoiceSet(-1);
    }
    mbWinTopWait();
    choice = mbWinTopChoiceGet();
    if (choice == 0) {
        CapEffRemoveCreate();
        HuPrcVSleep();
        mbAudFXPlay(MSM_SE_BRD00_17);
        mbObjDispSet(work->capObjId, FALSE);
        mbObjPosGet(work->capObjId, &playerPos);
        removePos = playerPos;
        removePosP = &removePos;
        CapEffRemoveAddAll(removePosP);
        omVibrate(work->playerNo, 20, 4, 4);
        // Some callers request the effect without removing an inventory capsule.
        if (!capsuleUseRemoveOnF) {
            mbPlayerCapsuleUseSet(work->capsuleNo);
            capsuleSlot = mbPlayerCapsuleFind(work->playerNo, work->capsuleNo);
            if (capsuleSlot != MB_MODEL_NONE) {
                mbPlayerCapsuleRemove(work->playerNo, capsuleSlot);
            }
        }
        do {
            HuPrcVSleep();
        } while (CapEffRemoveCheck());
        CapEffRemoveKill();
        HuPrcVSleep();
    } else if (oldObjId != MB_MODEL_NONE) {
        mbCapSelectResultSet(work->playerNo, oldObjId, result);
        work->capObjId = MB_MODEL_NONE;
    }
    if (work->primaryWinId != MB_MODEL_NONE) {
        mbWinKill(work->primaryWinId);
    }
    work->primaryWinId = MB_MODEL_NONE;
    if (work->secondaryWinId != MB_MODEL_NONE) {
        mbWinKill(work->secondaryWinId);
    }
    work->secondaryWinId = MB_MODEL_NONE;
    if (work->capObjId != MB_MODEL_NONE) {
        mbCapObjKill(work->capObjId);
    }
    work->capObjId = MB_MODEL_NONE;
    capsuleObjId = MB_MODEL_NONE;
    CapUseDeleteKill(work);
    if (choice == 0) {
        return TRUE;
    }
    return FALSE;
}

// Releases remaining preview models and optional windows after the discard prompt.
static void CapUseDeleteKill(CAP_USE_DELETE_WORK *work)
{
    if (work->capObjId != MB_MODEL_NONE) {
        mbCapObjKill(work->capObjId);
    }
    work->capObjId = MB_MODEL_NONE;
    if (work->auxModelId != MB_MODEL_NONE) {
        mbObjKill(work->auxModelId);
    }
    work->auxModelId = MB_MODEL_NONE;
    if (work->primaryWinId != MB_MODEL_NONE) {
        mbWinKill(work->primaryWinId);
    }
    work->primaryWinId = MB_MODEL_NONE;
    if (work->secondaryWinId != MB_MODEL_NONE) {
        mbWinKill(work->secondaryWinId);
    }
    work->secondaryWinId = MB_MODEL_NONE;
}

// Installs a board's supplied capsule list, retaining only entries enabled for distribution.
// Existing-save setup also clears the two capsule counters.
void mbCapListInit(CAPSULE_LIST *list)
{
    int entryIndex;
    int entryCount;

    for (entryIndex = 0, entryCount = 0; entryIndex < 33; entryIndex++) {
        if (list[entryIndex].id == -1) {
            break;
        }
        if (mbCapListExcludeCheck(list[entryIndex].id)) {
            capsuleList[entryCount] = list[entryIndex];
            entryCount++;
        }
    }
    capsuleList[entryCount].id = -1;
    if (!mbSaveNewF) {
        memset(capsuleNum, 0, sizeof(capsuleNum));
    }
}

// Capsule-event initialization loads the current board's distribution list and filters it.
// Existing-save setup also clears the two capsule counters.
void mbCapListRead(void)
{
    CAPSULE_LIST_FILE *file;
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    int i;
    int num;
    int boardNo;

    file = capsuleListFileTbl;
    i = 0;
    for (; file->boardNo >= 0; i++, file++) {
        boardNo = GwSystem.boardNo;
        if (file->boardNo == boardNo) {
            break;
        }
    }
    listBase = list = HuDataSelHeapReadNum(
        file->dataNo, HU_MEMNUM_OVL, HEAP_MODEL);
    i = 0;
    num = 0;
    for (; i < 33; i++, list++) {
        if (list->id == -1) {
            break;
        }
        if (mbCapListExcludeCheck(list->id)) {
            capsuleList[num] = *list;
            num++;
        }
    }
    capsuleList[num].id = -1;
    HuMemDirectFree(listBase);
    if (!mbSaveNewF) {
        memset(capsuleNum, 0, sizeof(capsuleNum));
    }
}

// Copies active distribution entries for a caller and returns their count.
// The terminator is written back to the internal list, rather than to the caller's copy.
int mbCapListCopy(CAPSULE_LIST *list)
{
    int entryIndex;

    for (entryIndex = 0; entryIndex < 32; entryIndex++) {
        if (capsuleList[entryIndex].id == -1) {
            break;
        }
        list[entryIndex] = capsuleList[entryIndex];
    }
    capsuleList[entryIndex].id = -1;
    return entryIndex;
}

// The capsule debug screen edits distribution entries and commits them on both triggers.
// A accepts a row edit, B restores it, and Y prints the edited list.
void mbCapListDebug(void)
{
    static GXColor winColor = { 0, 0, 144, 192 };
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    CAPSULE_LIST_DEFINE *listDefine = capsuleListDefineTbl;
    CAPSULE_LIST listTemp;
    u8 debugDataA[15][8];
    u8 debugDataB[15][8];
    s16 total;
    int copyNum;
    int num;
    int mode;
    int row;
    int col;
    int directionY;
    int color;
    int prevColor;
    int directionX;
    CAPSULE_LIST *listData;
    int i;
    int j;
    int x;
    int y;
    s8 *value;

    listData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    listBase = listData;
    list = listBase;
    memset(listBase, 0, sizeof(*listBase) * 33);
    i = 0;
    list = listBase;
    for (; i < 33; i++, list++) {
        list->id = -1;
    }
    {
        int num;

        for (num = 0; num < 32; num++) {
            if (capsuleList[num].id == -1) {
                break;
            }
            listBase[num] = capsuleList[num];
        }
        capsuleList[num].id = -1;
    }
    num = mode = 0;
    row = mode;
    col = row;
    memset(debugDataA, 0, sizeof(debugDataA));
    memset(debugDataB, 0, sizeof(debugDataB));

    do {
        printWin(4, 96, 630, 270, &winColor);
        x = 8;
        y = 100;
        color = -1;
        prevColor = color;
        i = 0;
        list = listBase;
        for (; i < 15; i++, list++) {
            value = (s8 *)list;
            x = 8;
            for (j = 0; j < 16; j++) {
                switch (mode) {
                    case 0:
                        if (row == i) {
                            color = 14;
                        } else {
                            color = 15;
                        }
                        break;

                    case 1:
                        if (row == i) {
                            if (col == j) {
                                color = 14;
                            } else {
                                color = 10;
                            }
                        } else {
                            color = 15;
                        }
                        break;
                }
                if (color != prevColor) {
                    fontcolor = color;
                }
                prevColor = color;
                if (j == 0) {
                    if (list->id != -1) {
                        print8((s16)x, (s16)y, 1.5f, "%s",
                            mbCapDebugNameGet(list->id));
                    } else {
                        print8((s16)x, (s16)y, 1.5f, "NULL");
                    }
                } else {
                    print8((s16)x, (s16)y, 1.5f, "%02d",
                        value[j]);
                }
                x += capsuleListColW[j];
            }
            y += 16;
        }
        x = 8;
        for (j = 0; j < 16; j++) {
            if (mode == 1 && j == col) {
                color = 14;
            } else {
                color = 13;
            }
            if (color != prevColor) {
                fontcolor = color;
            }
            prevColor = color;
            total = 0;
            i = 0;
            list = listBase;
            for (; i < 15; i++, list++) {
                if (list->id != -1) {
                    value = (s8 *)list;
                    total += value[j];
                }
            }
            if (j >= 4) {
                if (total < 100) {
                    print8((s16)x, (s16)y, 1.5f, "%02d", total);
                } else {
                    print8((s16)(x - 8), (s16)y, 1.5f, "%02d",
                        total);
                }
            }
            x += capsuleListColW[j];
        }
        directionY = 0;
        directionX = directionY;
        if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            directionX = 1;
        } else if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            directionX = -1;
        }
        if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            directionY = -1;
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            directionY = 1;
        }
        switch (mode) {
            case 0:
                if (directionY > 0) {
                    row++;
                } else if (directionY < 0) {
                    row--;
                }
                if (row >= 15) {
                    row = 0;
                } else if (row < 0) {
                    row = 14;
                }
                break;

            case 1:
                if (directionX > 0) {
                    col++;
                } else if (directionX < 0) {
                    col--;
                }
                if (col >= 16) {
                    col = 0;
                } else if (col < 0) {
                    col = 15;
                }
                value = (s8 *)&listBase[row];
                if (directionY > 0) {
                    value[col]--;
                } else if (directionY < 0) {
                    value[col]++;
                }
                if (col == 0) {
                    if (directionY < 0) {
                        while (!mbCapValidCheck(value[col]) && value[col] < 32) {
                            value[col]++;
                        }
                    } else if (directionY > 0) {
                        while (!mbCapValidCheck(value[col]) && value[col] > -1) {
                            value[col]--;
                        }
                    }
                    if (value[col] > 32) {
                        value[col] = 32;
                    } else if (value[col] < -1) {
                        value[col] = -1;
                    }
                } else {
                    if (value[col] > 99) {
                        value[col] = 99;
                    } else if (value[col] < 0) {
                        value[col] = 0;
                    }
                }
                break;
        }
        switch (mode) {
            case 0:
                if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                    memcpy(&listTemp, &listBase[row], sizeof(listTemp));
                    mode++;
                }
                break;

            case 1:
                if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                    mode = 0;
                } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                    memcpy(&listBase[row], &listTemp, sizeof(listTemp));
                    mode = 0;
                }
                break;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_Y) {
            OSReport("static CAPSULE_LIST capsuleList[] = { \n");
            i = 0;
            list = listBase;
            for (; i < 15; i++, list++) {
                if (list->id != -1) {
                    value = (s8 *)list;
                    for (j = 0, listDefine = capsuleListDefineTbl;
                         listDefine->capsuleNo != -1;
                         j++, listDefine++) {
                        if (listDefine->capsuleNo == list->id) {
                            break;
                        }
                    }
                    OSReport("  { %s, ", listDefine->name);
                    OSReport("{%d,%d,%d}, ", value[1], value[2], value[3]);
                    OSReport("{%d,%d,%d,%d,%d,%d,%d,%d}, ",
                        value[4], value[5], value[6], value[7], value[8],
                        value[9], value[10], value[11]);
                    OSReport("{%d,%d,%d,%d} }, \n",
                        value[12], value[13], value[14], value[15]);
                }
            }
            OSReport("\t{ CAPSULE_NULL, ");
            OSReport("{%d,%d,%d}, ", 0, 0, 0);
            OSReport("{%d,%d,%d,%d,%d,%d,%d,%d}, ",
                0, 0, 0, 0, 0, 0, 0, 0);
            OSReport("{%d,%d,%d,%d} }, \n", 0, 0, 0, 0);
            OSReport("};\n");
        }
        HuPrcVSleep();
    } while (!(HuPadBtn[0] & PAD_TRIGGER_R) ||
        !(HuPadBtn[0] & PAD_TRIGGER_L));

    {
        i = 0;
        copyNum = 0;
        list = listBase;
        for (; i < 33; i++, list++) {
            if (list->id != -1) {
                capsuleList[copyNum] = *list;
                copyNum++;
            }
        }
        capsuleList[copyNum].id = -1;
    }
    HuMemDirectFree(listBase);
}

void mbCapNumInc(int capsuleNo, int mode)
{
    capsuleNum[capsuleNo][mode]++;
}

static int capsuleNumColW[16] = {
    108, 32, 32, 48,
    32, 32, 32, 32,
    32, 32, 32, 48,
    32, 32, 32, 48,
};

// The capsule debug counter screen shows each entry's two counts and their sum.
// It waits for both controller-zero triggers before returning.
void mbCapNumDebug(void)
{
    static GXColor winColor = { 0, 0, 144, 192 };
    CAPSULE_LIST *list;
    float x;
    float y;
    int color;
    int i;
    int prevColor;
    int j;

    do {
        printWin(4, 96, 630, 270, &winColor);
        x = 8.0f;
        y = 100.0f;
        prevColor = -1;
        color = prevColor;
        i = 0;
        list = capsuleList;
        for (; i < 15; i++, list++) {
            x = 80.0f;
            for (j = 0; j < 4; j++) {
                if (color != prevColor) {
                    fontcolor = color;
                }
                prevColor = color;
                if (j == 0) {
                    if (list->id != -1) {
                        print8(x, y, 1.5f, "%s", mbCapDebugNameGet(list->id));
                    } else {
                        print8(x, y, 1.5f, "NULL");
                    }
                } else if (j == 3) {
                    print8(x, y, 1.5f, "%02d",
                        capsuleNum[list->id][0] + capsuleNum[list->id][1]);
                } else {
                    print8(x, y, 1.5f, "%02d", capsuleNum[list->id][j - 1]);
                }
                x += capsuleNumColW[j];
            }
            y += 16.0f;
        }
        HuPrcVSleep();
    } while (!(HuPadBtn[0] & PAD_TRIGGER_R) ||
        !(HuPadBtn[0] & PAD_TRIGGER_L));
}

s16 mbCapValueTypeGet(s16 value)
{
    return value & CAPSULE_VALUE_TYPE_MASK;
}

s16 mbCapMasuTypeGet(s16 masuId)
{
    return mbCapValueTypeGet(mbMasuCapsuleGet(masuId));
}

s16 mbCapValuePlayerGet(s16 value)
{
    return (value >> CAPSULE_VALUE_PLAYER_SHIFT) & CAPSULE_VALUE_TYPE_MASK;
}

s16 mbCapMasuPlayerGet2(s16 masuId)
{
    return mbMasuCapsuleGet(masuId) >> 8;
}

// Space-placement callers add an owner to the packed capsule value.
// Owner bits are ORed into the existing value, so this does not clear an earlier owner.
void mbCapMasuPlayerSet(s16 masuId, s16 playerNo)
{
    s16 capsuleValue = (s16)mbMasuCapsuleGet(masuId);

    capsuleValue |=
        (playerNo & CAPSULE_VALUE_TYPE_MASK) << CAPSULE_VALUE_PLAYER_SHIFT;
    mbMasuCapsuleSet(masuId, capsuleValue);
}

void mbCapMasuPlayerTypeSet(s16 masuId, s16 capsuleNo, s16 playerNo)
{
    mbMasuCapsuleSet(masuId, capsuleNo);
    mbCapMasuPlayerSet(masuId, playerNo);
}

s16 mbCapMasuPlayerGet(s16 masuId)
{
    return mbMasuCapsuleGet(masuId) >> 8;
}

s16 mbCapUseModeGet(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].useMode;
}

// Capsule rule callers test whether the type uses trap placement mode.
BOOL mbCapUseTrapCheck(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (mbCapUseModeGet(capsuleNo) == 2) {
        return TRUE;
    }
    return FALSE;
}

// The capsule shop reads a daytime or night/rank price, falling back to the base cost.
int mbCapBuyCostGet(s16 capsuleNo, s16 playerNo)
{
    int cost;
    int i;
    int costNo;

    cost = -1;
    if (!GwSystem.curTime) {
        costNo = 0;
    } else {
        switch (GwPlayer[playerNo].rank) {
            case 0:
                costNo = 1;
                break;
            case 1:
                costNo = 0;
                break;
            default:
                costNo = 2;
                break;
        }
    }
    for (i = 0; i < 32; i++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        if (capsuleNo == capsuleList[i].id) {
            if (capsuleList[i].cost[costNo] > 0) {
                cost = capsuleList[i].cost[costNo];
            }
            break;
        }
    }
    if (cost > 0) {
        return cost;
    }
    return mbCapCostGet(capsuleNo);
}

// Sale-price callers select the player's rank price, falling back to the base capsule cost.
int mbCapSellCostGet(s16 capsuleNo, s16 playerNo)
{
    int cost;
    int costNo;
    int i;

    cost = -1;
    switch (GwPlayer[playerNo].rank) {
        case 0:
            costNo = 2;
            break;
        case 1:
            costNo = 0;
            break;
        default:
            costNo = 1;
            break;
    }
    for (i = 0; i < 32; i++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        if (capsuleNo == capsuleList[i].id) {
            if (capsuleList[i].cost[costNo] > 0) {
                cost = capsuleList[i].cost[costNo];
            }
            break;
        }
    }
    if (cost > 0) {
        return cost;
    }
    return mbCapCostGet(capsuleNo);
}

int mbCapCostGet(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].cost;
}

BOOL mbCapListExcludeCheck(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].listFlag;
}

// Last-five-turn placement accepts an unoccupied blue/red space with no displayed trap.
BOOL mbCapThrowMasuCheck(int masuId)
{
    BOOL result;
    int masuType;
    u32 masuMAttr;
    int i;

    masuType = mbMasuTypeGet(masuId);
    masuMAttr = mbMasuMAttrGet(masuId);
    if (masuType != 1 && masuType != 2) {
        goto result_false;
    }
    if (mbCapMasuDispTypeGet(masuId) != 0 &&
        mbCapMasuDispTypeGet(masuId) != 1) {
        goto result_false;
    }
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        if (masuId == GwPlayer[i].masuId) {
            result = FALSE;
            goto done;
        }
    }
    result = TRUE;
    goto done;

result_false:
    result = FALSE;
done:
    return result;
}

// Computer selection starts with character weights, then adjusts for route and inventory.
// Modes return adjusted use weight, base weight, or the complementary discard weight.
int mbCapComChanceGet(int capsuleNo, int playerNo, int mode)
{
    CAPSULE_COM_CHANCE *data;
    int chance;
    int rank;
    int step;
    int step2;
    int step3;
    int step4;
    int i;
    s8 code;

    data = capsuleChanceTbl;
    capsuleSelectComBack = FALSE;
    i = 0;
    while (data->capsuleNo != -1) {
        if (data->capsuleNo == capsuleNo) {
            break;
        }
        i++;
        data++;
    }
    if (data->capsuleNo < 0) {
        return -1;
    }
    rank = GwPlayer[playerNo].charNo;
    if (rank > 10) {
        rank = 10;
    }
    chance = data->rank[rank].chance;
    code = data->rank[rank].code;
    step = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 7);
    switch (capsuleNo) {
        case 1:
            if (code == '*' && CapCheckComPath(playerNo, 15, 0)) {
                chance = 0;
            }
            break;
        case 2:
            step2 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 3);
            step3 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 5);
            step4 = mbMasuFind_TypeStepGet(GwPlayer[playerNo].masuId, 4);
            if ((step2 >= 1 && step2 <= 10) || (step3 >= 1 && step3 <= 10) ||
                (step4 >= 1 && step4 <= 10)) {
                if (code == 'a') {
                    chance = 80;
                } else if (code == 'b') {
                    chance = 50;
                }
            }
            break;
        case 3:
            if (code == '*' && CapCheckComPath(playerNo, 10, 1)) {
                chance = 80;
            }
            break;
        case 4:
            if (code == '*') {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    if (i != playerNo &&
                        GwPlayer[i].masuId == GwPlayer[playerNo].masuId) {
                        break;
                    }
                }
                if (i >= GW_PLAYER_MAX && !CapCheckComPath(playerNo, 10, 2)) {
                    chance = 0;
                }
            }
            if (step >= 1 && step <= 10) {
                chance = 0;
            }
            break;
        case 5:
            if (code == '*') {
                for (i = 0; i < GW_PLAYER_MAX; i++) {
                    if (i != playerNo) {
                        step2 = mbMasuFind_TypeStepGet(GwPlayer[i].masuId, 7);
                        if (step2 >= 1 && step2 <= 10) {
                            break;
                        }
                    }
                }
                if (i < GW_PLAYER_MAX) {
                    chance = 80;
                }
            }
            break;
        case 6:
            if (code == '*' && mbPlayerCoinGet(playerNo) <= 20) {
                chance = 0;
            }
            break;
        case 11:
            if (code == '*' && mbPlayerCoinGet(playerNo) < 10) {
                chance = 0;
            }
            break;
        case 22:
            if (code == '*' && step >= 0 && step <= 5 &&
                mbCapSelectMasuBackNum(GwPlayer[playerNo].masuId) > 0) {
                chance = 80;
                capsuleSelectComBack = TRUE;
            }
            break;
        case 23:
            if (code == '*' && step >= 0 && step <= 5 &&
                mbCapSelectMasuBackNum(GwPlayer[playerNo].masuId) > 0) {
                chance = 80;
                capsuleSelectComBack = TRUE;
            }
            break;
        case 30:
            if (code == '*' &&
                mbPlayerCapsuleNumGet(playerNo) >= mbPlayerCapsuleMaxGet()) {
                chance = 30;
            }
            if (mode == 0 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 0;
            }
            if (mode == 2 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 100;
            }
            break;
        case 31:
            if (code == '*' &&
                mbPlayerCapsuleNumGet(playerNo) >= mbPlayerCapsuleMaxGet()) {
                chance = 30;
            }
            if (mode == 0 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 0;
            }
            if (mode == 2 &&
                mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet()) {
                chance = 100;
            }
            break;
    }
    switch (mode) {
        case 0:
            return chance;
        case 1:
            return data->rank[rank].chance;
        case 2:
            return 100 - chance;
        default:
            return 0;
    }
}

static int capsuleThrowTbl[8] = { 10, 11, 12, 13, 25, 15, 16, 17 };

static int capsuleTrapTbl[5] = { 20, 21, 22, 23, 24 };

static int capsuleChanceWeightTbl[3][4][5] = {
    {
        { 70, 15, 15 },
        { 70, 20, 10 },
        { 70, 20, 0, 10 },
        { 50, 30, 0, 20 },
    },
    {
        { 40, 30, 30 },
        { 30, 30, 20, 20 },
        { 10, 40, 10, 30, 10 },
        { 10, 40, 0, 30, 20 },
    },
    {
        { 30, 30, 30, 10 },
        { 20, 20, 20, 30, 10 },
        { 10, 30, 0, 40, 20 },
        { 10, 20, 0, 40, 30 },
    },
};

static int capsuleMaxTurnTbl[9] = { 10, 15, 20, 25, 30, 35, 40, 45, 50 };

static int capsuleTurnTbl[9][2] = {
    { 3, 6 },
    { 5, 10 },
    { 5, 15 },
    { 8, 16 },
    { 10, 20 },
    { 10, 20 },
    { 13, 26 },
    { 15, 30 },
    { 15, 35 },
};

// Computer capsule weights search reachable forward routes within a displayed-space budget.
// Modes seek an opponent's capsule 23, any opponent-owned trap, or another player.
static BOOL CapCheckComPath(int playerNo, int max, int mode)
{
    s16 masuStack[32];
    s16 maxStack[32];
    s8 visited[256];
    int maxWork;
    int masuId;
    int nextMasu;
    int startMasu;
    s16 stackNum;
    int linkNum;
    int i;
    BOOL endF;

    masuId = startMasu = GwPlayer[playerNo].masuId;
    maxWork = max;
    {
        int linkNum;

        linkNum = 0;
    }
    endF = FALSE;
    stackNum = 0;
    memset(visited, 0, sizeof(visited));
    do {
    loopTop:
        visited[masuId] = TRUE;
        if (masuId != startMasu &&
            ((mbMasuAttrGet(masuId) & mbBranchAttrGet()) ||
             (mbMasuMAttrGet(masuId) & mbBranchMAttrGet()))) {
            maxWork = 0;
        } else {
            if (mbMasuDispCheck(masuId)) {
                maxWork--;
            }
            if (startMasu != masuId) {
                switch (mode) {
                case 0:
                    if (mbCapMasuDispTypeGet(masuId) == 2 &&
                        mbCapValueTypeGet(mbMasuCapsuleGet(masuId)) == 23 &&
                        mbCapMasuPlayerGet2(masuId) != playerNo) {
                        return TRUE;
                    }
                    break;
                case 1:
                    if (mbCapMasuDispTypeGet(masuId) == 2 &&
                        mbCapMasuPlayerGet2(masuId) != playerNo) {
                        return TRUE;
                    }
                    break;
                case 2:
                    for (i = 0; i < GW_PLAYER_MAX; i++) {
                        if (i != playerNo && GwPlayer[i].masuId == masuId) {
                            break;
                        }
                    }
                    if (i < GW_PLAYER_MAX) {
                        return TRUE;
                    }
                    break;
                }
            }
            if (mbMasuLinkNumGet(masuId) <= 0 || maxWork <= 0) {
                maxWork = 0;
            } else {
                linkNum = mbMasuLinkNumGet(masuId);
                for (i = 0; i < linkNum; i++) {
                    nextMasu = mbMasuLinkGet(masuId, i);
                    if (!visited[nextMasu]) {
                        if (i < mbMasuLinkNumGet(masuId) - 1) {
                            masuStack[stackNum] = masuId;
                            maxStack[stackNum] = maxWork;
                            stackNum++;
                        }
                        masuId = nextMasu;
                        break;
                    }
                }
                if (i >= linkNum) {
                    maxWork = 0;
                }
                if (mbMasuLinkNumGet(masuId) <= 0) {
                    visited[masuId] = TRUE;
                }
                if (maxWork > 0) {
                    goto loopTop;
                }
            }
        }
        if (stackNum <= 0) {
            endF = TRUE;
        } else {
            stackNum--;
            masuId = masuStack[stackNum];
            maxWork = maxStack[stackNum];
        }
    } while (!endF);
    return FALSE;
}

static inline s16 CapUseModeGetInline(s16 capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].useMode;
}

// Capsule selection chooses a computer player's inventory index by adjusted use weights.
// It can decline a choice, follows tutorial control, and remembers a backward throw choice.
int mbCapSelectComGet(int playerNo, int *capsuleTbl, int capsuleNum)
{
    CAPSULE_COM_CHOICE_BACK temp;
    CAPSULE_COM_CHOICE_BACK choice[10];
    CAPSULE_COM_CHOICE_BACK *choiceP;
    float random;
    int chance;
    int i;
    int j;

    if (_CheckFlag(FLAG_BOARD_TUTORIAL)) {
        i = mbTutorialCall(15);
        return i;
    }
    if (capsuleNum <= 0) {
        return -1;
    }
    if (GwSystem.turnNo >= GwSystem.turnMax && mbPlayerCoinGet(playerNo) >= 20) {
        for (i = 0; i < capsuleNum; i++) {
            if (capsuleTbl[i] == 6) {
                break;
            }
        }
        if (i < capsuleNum) {
            return i;
        }
    }
    if (mbPlayerCapsuleNumGet(playerNo) < mbPlayerCapsuleMaxGet() &&
        (100.0f * MBCapsuleEffRandF()) < 30.0f) {
        return -1;
    }
    i = 0;
    choiceP = choice;
    for (; i < capsuleNum; i++, choiceP++) {
        choiceP->index = i;
        choiceP->capsuleNo = capsuleTbl[i];
        choiceP->chance = mbCapComChanceGet(choiceP->capsuleNo, playerNo, 0);
        if (capsuleSelectComBack) {
            choiceP->back = TRUE;
        } else {
            choiceP->back = FALSE;
        }
        if ((CapUseModeGetInline(choiceP->capsuleNo) == 1 ||
                CapUseModeGetInline(choiceP->capsuleNo) == 2) &&
            mbCapSelectMasuNum(GwPlayer[playerNo].masuId) <= 0) {
            choiceP->chance = 0;
        }
    }
    for (i = 0; i < capsuleNum; i++) {
        for (j = i + 1; j < capsuleNum; j++) {
            if (choice[i].chance < choice[j].chance) {
                temp = choice[i];
                choice[i] = choice[j];
                choice[j] = temp;
            }
        }
    }
    j = capsuleNum;
    capsuleNum = 0;
    chance = 0;
    for (i = 0; i < j; i++) {
        if (choice[i].chance > 0) {
            chance += choice[i].chance;
            capsuleNum++;
        }
    }
    if (capsuleNum <= 0) {
        return -1;
    }
    random = (float)chance * MBCapsuleEffRandF();
    i = 0;
    choiceP = choice;
    for (; i < capsuleNum; i++, choiceP++) {
        random -= choiceP->chance;
        if (random <= 0.0f) {
            break;
        }
    }
    if (i >= capsuleNum) {
        return -1;
    }
    if (choiceP->back) {
        capsuleSelectComBack = TRUE;
    } else {
        capsuleSelectComBack = FALSE;
    }
    return choiceP->index;
}

// The discard selection chooses the inventory index with the greatest discard weight.
int mbCapSelectDeleteComGet(int playerNo, int *capsuleTbl, int capsuleNum)
{
    CAPSULE_COM_CHOICE temp;
    CAPSULE_COM_CHOICE choice[10];
    CAPSULE_COM_CHOICE *choiceP;
    int i;
    int j;

    if (capsuleNum <= 0) {
        return -1;
    }
    i = 0;
    choiceP = choice;
    for (; i < capsuleNum; i++, choiceP++) {
        choiceP->index = i;
        choiceP->capsuleNo = capsuleTbl[i];
        choiceP->chance = mbCapComChanceGet(choiceP->capsuleNo, playerNo, 2);
    }
    for (i = 0; i < capsuleNum; i++) {
        for (j = i + 1; j < capsuleNum; j++) {
            if (choice[i].chance < choice[j].chance) {
                temp = choice[i];
                choice[i] = choice[j];
                choice[j] = temp;
            }
        }
    }
    return choice[0].index;
}

// Board event callers fill blue/red spaces with throw capsules and selected or random owners.
// A negative type picks randomly; checkF zero preserves spaces that already hold a capsule.
void mbCapRandomThrowAdd(int capsuleNo, int playerNo, int checkF)
{
    int masuId;
    int masuType;
    int value;
    int owner;

    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        masuType = mbMasuTypeGet(masuId);
        if (checkF == 0 && mbMasuCapsuleGet(masuId) != -1) {
            continue;
        }
        if (masuType != 1 && masuType != 2) {
            continue;
        }
        if (capsuleNo >= 0) {
            value = capsuleThrowTbl[(capsuleNo - 10) % 8];
        } else {
            value = capsuleThrowTbl[mbRandMod(8)];
        }
        if (playerNo >= 0) {
            owner = playerNo;
        } else {
            owner = mbRandMod(4);
        }
        mbCapMasuCapsuleSet(masuId, value, owner);
    }
}

// Board event callers fill blue/red spaces with traps and selected or random owners.
// A negative type picks randomly; checkF zero preserves spaces that already hold a capsule.
void mbCapRandomTrapAdd(int capsuleNo, int playerNo, int checkF)
{
    int masuId;
    int masuType;
    int value;
    int owner;

    for (masuId = 1; masuId < mbMasuNumGet(); masuId++) {
        masuType = mbMasuTypeGet(masuId);
        if (checkF == 0 && mbMasuCapsuleGet(masuId) != -1) {
            continue;
        }
        if (masuType != 1 && masuType != 2) {
            continue;
        }
        if (capsuleNo >= 0) {
            value = capsuleTrapTbl[(capsuleNo - 20) % 5];
        } else {
            value = capsuleTrapTbl[mbRandMod(5)];
        }
        if (playerNo >= 0) {
            owner = playerNo;
        } else {
            owner = mbRandMod(4);
        }
        mbCapMasuCapsuleSet(masuId, value, owner);
    }
}

// Throw landing and board population store the capsule type and owner on a space.
// Types above the duel capsule and the bone capsule are ignored by this placement path.
void mbCapMasuCapsuleSet(int masuId, int capsuleNo, int playerNo)
{
    s16 value;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo <= CAPSULE_KETTOU) {
        switch (capsuleNo) {
            case CAPSULE_HONE:
                return;
            case CAPSULE_KOOPA:
                return;
            default:
                mbMasuCapsuleSet(masuId, (s16)capsuleNo);
                value = mbMasuCapsuleGet(masuId);
                value |= ((s16)playerNo & CAPSULE_VALUE_TYPE_MASK)
                    << CAPSULE_VALUE_PLAYER_SHIFT;
                mbMasuCapsuleSet(masuId, value);
                switch (capsuleNo) {
                    case CAPSULE_BOMHEI:
                        break;
                }
                break;
        }
    }
}

// After player or automatic landing, bone and Koopa capsules show the special ending message.
static void CapThrowEndWin(int unusedMasuId, int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);

    switch (capsuleNo) {
        case CAPSULE_HONE:
            mbWinCreate(2,
                MESSNUM(
                    MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_END_SPECIAL),
                -1);
            mbWinTopWait();
            break;

        case CAPSULE_KOOPA:
            mbWinCreate(2,
                MESSNUM(
                    MESS_CAPSULE_EX99, CAPSULE_EX99_MESSAGE_END_SPECIAL),
                -1);
            mbWinTopWait();
            break;
    }
}

int mbCapFileGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return mbBoardDataNumGet(capsuleData[capsuleNo].file);
}

int mbCapDescMesGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].descMes;
}

int mbCapUseMesGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].useMes;
}

char *mbCapDebugNameGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].debugName;
}

int mbCapMasuPatGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].masuPat;
}

int mbCapColorGet(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    return capsuleData[capsuleNo].color;
}

int mbCapUseCostGet(void)
{
    return 0;
}

// Capsule rule callers test whether this type is used directly on the player.
BOOL mbCapUseCheck(int capsuleNo)
{
    if (mbCapUseModeGet(capsuleNo) == 0) {
        return TRUE;
    }
    return FALSE;
}

// Distribution and debug callers accept capsule types that have a model file assigned.
BOOL mbCapValidCheck(int capsuleNo)
{
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo < 0 || capsuleNo >= CAPSULE_MAX) {
        return FALSE;
    }
    if (capsuleData[capsuleNo].file == 0) {
        return FALSE;
    }
    return TRUE;
}

// Placement and board effects classify blue/red spaces as empty, a throw capsule, or a trap.
// Capsules on other space types are hidden from this classification.
s16 mbCapMasuDispTypeGet(s16 masuId)
{
    s16 capsuleNo;

    capsuleNo = mbMasuCapsuleGet(masuId);
    capsuleNo = mbCapValueTypeGet(capsuleNo);
    if (capsuleNo == CAPSULE_VALUE_NONE) {
        return 0;
    }
    if (mbMasuTypeGet(masuId) != 1 && mbMasuTypeGet(masuId) != 2) {
        return 0;
    }
    if (mbCapUseModeGet(capsuleNo) == 1) {
        return 1;
    }
    return 2;
}

int mbCapSelectMasuNum(int masuId)
{
    return mbCapSelectMasuFrontNum(masuId)
        + mbCapSelectMasuBackNum(masuId);
}

// Computer selection counts eligible capsule destinations within five forward spaces.
// Enables selection completion before collecting the route flags.
int mbCapSelectMasuFrontNum(int masuId)
{
    s16 *masuFlagData;
    s16 *masuFlag;
    s16 i;
    s16 num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    memset(masuFlag, 0, sizeof(s16) * 256);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddFront(masuFlag, masuId, 5);
    for (i = 0, num = 0; i < 256; i++) {
        if (masuFlag[i] & 1) {
            num++;
        }
    }
    HuMemDirectFree(masuFlag);
    return num;
}

// Computer selection counts eligible capsule destinations within five backward spaces.
// Enables selection completion before collecting the route flags.
int mbCapSelectMasuBackNum(int masuId)
{
    s16 *masuFlagData;
    s16 *masuFlag;
    s16 i;
    s16 num;

    masuFlagData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(s16) * 256, HU_MEMNUM_OVL);
    masuFlag = masuFlagData;
    memset(masuFlag, 0, sizeof(s16) * 256);
    capsuleMasuSelectEndF = TRUE;
    CapSelectMasuAddBack(masuFlag, masuId, 5);
    for (i = 0, num = 0; i < 256; i++) {
        if (masuFlag[i] & 1) {
            num++;
        }
    }
    HuMemDirectFree(masuFlag);
    return num;
}

// Capsule generation collects the valid types below CAPSULE_VALID_LIST_MAX for its caller.
int mbCapValidListGet(int *list)
{
    int i;
    int num;

    i = 0;
    num = 0;
    for (; i < CAPSULE_VALID_LIST_MAX; i++) {
        if (mbCapValidCheck(i)) {
            list[num] = i;
            num++;
        }
    }
    return num;
}

// Capsule generation selects a valid type by rank and the current part of the game.
// Negative ranks and empty weight categories fall back to an unrestricted random type.
int mbCapNextGet(int rank)
{
    int i;
    int idx;
    int inIdx;
    int outIdx;
    int num;
    int *chanceTbl;
    int gamePart;
    int totalChance;
    int swappedCapsule;
    int turnSeg;
    int code;
    int chance;
    int list[CAPSULE_VALID_LIST_MAX];
    int outList[CAPSULE_VALID_LIST_MAX];
    CAPSULE_TURN_DATA turnData[5];

    num = mbCapValidListGet(list);
    if (rank < 0) {
        for (i = 0; i < num * 5; i++) {
            inIdx = i % num;
            outIdx = mbRandMod(num);
            if (inIdx != outIdx) {
                swappedCapsule = list[inIdx];
                list[inIdx] = list[outIdx];
                list[outIdx] = swappedCapsule;
            }
        }
        return list[mbRandMod(num)];
    }
    for (i = 0; i < 8U; i++) {
        if (GwSystem.turnMax <= capsuleMaxTurnTbl[i]) {
            break;
        }
    }
    if (i < 9U) {
        turnSeg = i;
        for (i = 0; i < 2; i++) {
            if (GwSystem.turnNo <= capsuleTurnTbl[turnSeg][i]) {
                break;
            }
        }
        gamePart = i;
    } else {
        if (GwSystem.turnNo <= GwSystem.turnMax / 3) {
            gamePart = 0;
        } else if (GwSystem.turnNo <= (GwSystem.turnMax / 3) * 2) {
            gamePart = 1;
        } else {
            gamePart = 2;
        }
    }
    if (rank > 3) {
        rank = 3;
    }
    chanceTbl = &capsuleChanceWeightTbl[gamePart][rank][0];
    for (i = 0, totalChance = 0, idx = 0; i < 5; i++) {
        if (chanceTbl[i] > 0) {
            turnData[idx].code = i + 'A';
            totalChance += chanceTbl[i];
            turnData[idx].chance = totalChance;
            idx++;
        }
    }
    chance = MBCapsuleEffRandF() * totalChance;
    for (i = 0; i < idx; i++) {
        if (chance < turnData[i].chance) {
            break;
        }
    }
    if (i >= idx) {
        i = 0;
    }
    code = turnData[i].code;
    for (i = 0, idx = 0; i < num; i++) {
        if (code == capsuleData[list[i]].code) {
            outList[idx] = list[i];
            idx++;
        }
    }
    if (idx <= 0) {
        return mbCapNextGet(-1);
    }
    for (i = 0; i < idx * 5; i++) {
        inIdx = i % idx;
        outIdx = mbRandMod(idx);
        if (inIdx != outIdx) {
            swappedCapsule = outList[inIdx];
            outList[inIdx] = outList[outIdx];
            outList[outIdx] = swappedCapsule;
        }
    }
    i = mbRandMod(idx);
    return outList[i];
}

// Landing on a capsule space selects a pickup using rank and the first or second game half.
// Each copy already carried by the player halves that capsule's selection weight.
int mbCapMasuNextGet(int playerNo)
{
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    CAPSULE_LIST *listWork;
    CAPSULE_LIST swappedEntry;
    int weightNo;
    int capsuleNo;
    int count;
    int i;
    int j;
    int listNo;
    int otherListNo;
    int total;

    if (GwSystem.turnNo <= GwSystem.turnMax / 2) {
        weightNo = GwPlayer[playerNo].rank;
    } else {
        weightNo = GwPlayer[playerNo].rank + 4;
    }
    list = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    listBase = list;
    listWork = listBase;
    i = 0;
    count = 0;
    total = 0;
    for (; i < 33; i++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        if (capsuleList[i].weight[weightNo] > 0) {
            *listWork = capsuleList[i];
            for (j = 0; j < mbPlayerCapsuleNumGet(playerNo); j++) {
                if (listWork->id == mbPlayerCapsuleGet(playerNo, j)) {
                    listWork->weight[weightNo] /= 2;
                }
            }
            total += listWork->weight[weightNo];
            listWork++;
            count++;
        }
    }
    if (count >= 2) {
        for (i = 0; i < 256; i++) {
            listNo = mbRandMod(count);
            otherListNo = mbRandMod(count);
            if (listNo != otherListNo) {
                swappedEntry = listBase[listNo];
                listBase[listNo] = listBase[otherListNo];
                listBase[otherListNo] = swappedEntry;
            }
        }
    } else {
        if (count == 1) {
            capsuleNo = listBase[0].id;
        } else {
            capsuleNo = -1;
        }
        goto cleanup;
    }
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (listBase[i].weight[weightNo] >
                listBase[j].weight[weightNo]) {
                swappedEntry = listBase[i];
                listBase[i] = listBase[j];
                listBase[j] = swappedEntry;
            }
        }
    }
    capsuleNo = (int)((float)total * MBCapsuleEffRandF());
    for (i = 0; i < count; i++) {
        capsuleNo -= listBase[i].weight[weightNo];
        if (capsuleNo <= 0) {
            break;
        }
    }
    if (i < count) {
        capsuleNo = listBase[i].id;
    } else {
        capsuleNo = listBase[0].id;
    }
cleanup:
    HuMemDirectFree(listBase);
    return capsuleNo;
}

// Shop stock generation chooses a capsule while excluding the two preceding offers.
// Eligibility and final selection use shop weights; the total uses adjusted pickup weights.
static int CapShopNextGet(int playerNo, int firstExcludedCapsule, int secondExcludedCapsule)
{
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    CAPSULE_LIST *listWork;
    CAPSULE_LIST swappedEntry;
    int rank;
    int capsuleNo;
    int count;
    int i;
    int j;
    int listNo;
    int otherListNo;
    int total;

    rank = GwPlayer[playerNo].rank;
    list = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    listBase = list;
    listWork = listBase;
    i = 0;
    count = 0;
    total = 0;
    for (; i < 33; i++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        if (capsuleList[i].id == firstExcludedCapsule ||
            capsuleList[i].id == secondExcludedCapsule) {
            continue;
        }
        if (*(&capsuleList[i].weight[8] + rank) <= 0) {
            continue;
        }
        *listWork = capsuleList[i];
        for (j = 0; j < mbPlayerCapsuleNumGet(playerNo); j++) {
            if (listWork->id == mbPlayerCapsuleGet(playerNo, j)) {
                listWork->weight[rank] /= 2;
            }
        }
        total += listWork->weight[rank];
        listWork++;
        count++;
    }
    if (count >= 2) {
        for (i = 0; i < 256; i++) {
            listNo = mbRandMod(count);
            otherListNo = mbRandMod(count);
            if (listNo != otherListNo) {
                swappedEntry = listBase[listNo];
                listBase[listNo] = listBase[otherListNo];
                listBase[otherListNo] = swappedEntry;
            }
        }
    } else {
        if (count == 1) {
            capsuleNo = listBase[0].id;
        } else {
            capsuleNo = -1;
        }
        goto cleanup;
    }
    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            if (*(&listBase[i].weight[8] + rank) >
                *(&listBase[j].weight[8] + rank)) {
                swappedEntry = listBase[i];
                listBase[i] = listBase[j];
                listBase[j] = swappedEntry;
            }
        }
    }
    capsuleNo = (int)((float)total * MBCapsuleEffRandF());
    for (i = 0; i < count; i++) {
        capsuleNo -= *(&listBase[i].weight[8] + rank);
        if (capsuleNo <= 0) {
            break;
        }
    }
    if (i < count) {
        capsuleNo = listBase[i].id;
    } else {
        capsuleNo = listBase[0].id;
    }
cleanup:
    HuMemDirectFree(listBase);
    return capsuleNo;
}

// A shop visit fills the caller's stock list with up to three distinct weighted offers.
int mbCapShopListGet(int playerNo, CAPSULE_LIST *list)
{
    int capsuleNo[CAPSULE_VALID_LIST_MAX];
    int i;
    int j;

    capsuleNo[0] = CapShopNextGet(playerNo, -1, -1);
    capsuleNo[1] = CapShopNextGet(playerNo, capsuleNo[0], -1);
    capsuleNo[2] =
        CapShopNextGet(playerNo, capsuleNo[0], capsuleNo[1]);
    for (i = 0; i < 3; i++) {
        if (capsuleNo[i] == -1) {
            break;
        }
        for (j = 0; j < 33; j++) {
            if (capsuleList[j].id == -1) {
                break;
            }
            if (capsuleList[j].id == capsuleNo[i]) {
                *list = capsuleList[j];
                list++;
                break;
            }
        }
        if (j >= 33) {
            break;
        }
    }
    return i;
}

// Last-five-turn capsule placement reads up to maxNum IDs from a shuffled capsule list.
// Output stops at an encountered -1 ID rather than at the number of copied entries.
int mbCapRandomListGet(int *capsuleListOut, int maxNum)
{
    CAPSULE_LIST *listBase;
    CAPSULE_LIST *list;
    int count;
    CAPSULE_LIST *listWork;
    CAPSULE_LIST swappedEntry;
    int i;
    int listNo;
    int otherListNo;
    int num;

    list = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*listBase) * 33, HU_MEMNUM_OVL);
    listBase = list;
    listWork = listBase;
    memset(listBase, 0, sizeof(*listBase) * 33);
    i = 0;
    count = 0;
    for (; i < 33; i++, listWork++) {
        if (capsuleList[i].id == -1) {
            break;
        }
        *listWork = capsuleList[i];
        count++;
    }
    if (count >= 2) {
        for (i = 0; i < 256; i++) {
            listNo = mbRandMod(count);
            otherListNo = mbRandMod(count);
            if (listNo != otherListNo) {
                swappedEntry = listBase[listNo];
                listBase[listNo] = listBase[otherListNo];
                listBase[otherListNo] = swappedEntry;
            }
        }
    }
    i = 0;
    num = 0;
    for (; i < maxNum; i++) {
        if (listBase[i].id == -1) {
            break;
        }
        capsuleListOut[i] = listBase[i].id;
        num++;
    }
    HuMemDirectFree(listBase);
    return num;
}

// Capsule use and throwing effects roll bonus coins from the player's rank and use mode.
// A failed roll returns zero; lower-ranked players have larger chances and coin ranges.
int mbCapBonusCoinNumGet(int playerNo, int capsuleNo)
{
    int bonus;
    int rank;

    rank = 0;
    bonus = 0;
    switch (GwPlayer[playerNo].rank) {
        case 0:
            rank = 0;
            break;

        case 1:
            rank = 1;
            break;

        case 2:
            rank = 2;
            break;

        case 3:
            rank = 3;
            break;
    }
    switch (mbCapUseModeGet(capsuleNo)) {
        case 0:
            if ((100.0f * MBCapsuleEffRandF()) <
                (20.0f + (10 * rank))) {
                bonus = mbRandMod(rank + 1) + 1;
            }
            break;

        case 1:
            if ((100.0f * MBCapsuleEffRandF()) <
                (40.0f + (10 * rank))) {
                bonus = mbRandMod(rank + 3) + 5;
            }
            break;

        case 2:
            if ((100.0f * MBCapsuleEffRandF()) <
                (40.0f + (10 * rank))) {
                bonus = mbRandMod(rank + 2) + 3;
            }
            break;
    }
    return bonus;
}

// Capsule selection and shop displays create a paused, instant-text description window.
// Packed owner bits are discarded, and the window is moved eight pixels above its default.
int mbCapDescWinCreate(int capsuleNo)
{
    HuVec2f pos;
    int winId;

    capsuleNo = mbCapValueTypeGet(capsuleNo);
    winId = mbWinCreate(6, mbCapDescMesGet(capsuleNo), -1);
    mbWinMesSpeedSet(winId, 0);
    mbWinPause(winId);
    mbWinPosGet(winId, &pos);
    pos.y -= 8.0f;
    mbWinPosSet(winId, (s16)pos.x, (s16)pos.y);
    return winId;
}

// Board setup clears capsule model tracking and creates two hidden linked model sources.
void mbCapInit(void)
{
    int i;
    int j;
    int objId;
    CAPSULE_OBJ_COLOR *objColorData;
    s16 *borderObjData;
    s16 *borderId;

    objColorData = capsuleObjColorData;
    memset(objColorData, 0, sizeof(capsuleObjColorData));
    for (i = 0; i < 6; i++) {
        capsuleObjBorderId[i] = MB_MODEL_NONE;
    }
    capsuleBorderObjId = borderObjData = HuMemDirectMallocNum(
        HEAP_HEAP, 768, HU_MEMNUM_OVL);
    borderId = borderObjData;
    memset(capsuleBorderObjId, 0, 768);
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 64; j++, borderId++) {
            *borderId = MB_MODEL_NONE;
        }
    }
    for (i = 0; i < 8; i++) {
        capsuleObjData[i].objId = MB_MODEL_NONE;
    }
    objId = mbObjCreate(
        DATANUM(DATA_capsule, CAPSULE_DATA_INIT_MODEL_0), NULL, TRUE);
    mbObjDispSet(objId, FALSE);
    objId = mbObjCreate(
        DATANUM(DATA_capsule, CAPSULE_DATA_INIT_MODEL_1), NULL, TRUE);
    mbObjDispSet(objId, FALSE);
}

// Capsule selection and use animations create an item model with its category border.
// The cancel item instead gets two texture animations and a 1.5 scale; specialF is ignored.
int mbCapObjCreate(int capsuleNo, BOOL specialF)
{
    ANIMDATA *anim;
    Mtx mtx;
    int objId;
    int i;

    if (capsuleNo == CAPSULE_YAMERU) {
        int capValue;
        s16 capValueShort;
        int fileNo;
        u32 file;

        for (i = 0; i < 8; i++) {
            if (capsuleObjData[i].objId == MB_MODEL_NONE) {
                break;
            }
        }
        capValue = capsuleNo;
        capValueShort = (s16)capValue & CAPSULE_VALUE_TYPE_MASK;
        capValue = capValueShort;
        fileNo = mbBoardDataNumGet(capsuleData[capValue].file);
        file = fileNo;
        objId = (s16)mbObjCreate(file, NULL, FALSE);
        capsuleObjData[i].objId = objId;
        anim = capsuleObjData[i].anim = HuSprAnimRead(HuDataSelHeapReadNum(
            DATANUM(DATA_capsule, CAPSULE_DATA_OBJ_ANIM),
            HU_MEMNUM_OVL, HEAP_MODEL));
        capsuleObjData[i].textureAnimId = Hu3DAnimCreate(
            anim, mbObjModelIDGet(objId), "S3TCys77120");
        Hu3DAnmNoSet(capsuleObjData[i].textureAnimId, 0);
        capsuleObjData[i].linkedTextureAnimId = Hu3DAnimLink(
            capsuleObjData[i].textureAnimId, mbObjModelIDGet(objId),
            "S3TCys77121");
        Hu3DAnmNoSet(capsuleObjData[i].linkedTextureAnimId, 0);
        PSMTXScale(mtx, 1.5f, 1.5f, 1.5f);
        mbObjMtxSet(objId, &mtx);
        return objId;
    } else {
        int capValue;
        s16 capValueShort;
        int fileNo;
        u32 file;

        capValue = capsuleNo;
        capValueShort = (s16)capValue & CAPSULE_VALUE_TYPE_MASK;
        capValue = capValueShort;
        fileNo = mbBoardDataNumGet(capsuleData[capValue].file);
        file = fileNo;
        objId = (s16)mbObjCreate(file, NULL, FALSE);
        mbCapObjBorderCreate(objId, capsuleNo);
        return objId;
    }
}

extern int capsuleBorderFileTbl[6];

void mbCapObjBorderKill(int objId);

// Item-model creation hooks the capsule category's shared border to its center object.
// Models without a center return -1; categories without border assets also have no border.
int mbCapObjBorderCreate(int objId, int capsuleNo)
{
    HSF_DATA *hsf;
    HSF_OBJECT *object;
    HSF_OBJECT *objectP;
    char name[HSF_OBJNAME_MAX_LEN];
    int i;
    int j;
    int groupNo;
    int modelId;
    s16 *borderId;

    modelId = mbObjModelIDGet(objId);
    hsf = Hu3DData[modelId].hsf;
    object = hsf->object;
    strcpy(name, MakeObjectName((s8 *)"center"));
    for (i = 0; i < hsf->objectNum; i++, object++) {
        objectP = object;
        if (objectP->constData && strcmp(name, objectP->name) == 0) {
            break;
        }
    }
    if (i >= hsf->objectNum) {
        return -1;
    }
    groupNo = capsuleNo / 10;
    if (capsuleObjBorderId[groupNo] < 0 && capsuleBorderFileTbl[groupNo] != 0) {
        capsuleObjBorderId[groupNo] = mbObjCreate(capsuleBorderFileTbl[groupNo], NULL, TRUE);
    }
    if (capsuleObjBorderId[groupNo] >= 0) {
        mbObjHookSet(objId, "center", capsuleObjBorderId[groupNo]);
        borderId = &capsuleBorderObjId[groupNo * 64];
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId == -1) {
                *borderId = objId;
                break;
            }
        }
    }
    return capsuleObjBorderId[groupNo];
}

// Capsule selection and use cleanup release an item model and its associated visual assets.
// Cancel-item models own texture animations; ordinary models release their border reference.
void mbCapObjKill(int objId)
{
    int i;

    for (i = 0; i < 8; i++) {
        if (objId == capsuleObjData[i].objId) {
            break;
        }
    }
    if (i < 8) {
        Hu3DAnimKill(capsuleObjData[i].textureAnimId);
        Hu3DAnimKill(capsuleObjData[i].linkedTextureAnimId);
        HuSprAnimKill(capsuleObjData[i].anim);
        mbObjKill(capsuleObjData[i].objId);
        capsuleObjData[i].objId = MB_MODEL_NONE;
    } else {
        mbCapObjBorderKill(objId);
        mbObjKill(objId);
    }
}

// Item-model cleanup removes its border reference and releases borders with no remaining users.
void mbCapObjBorderKill(int objId)
{
    s16 *borderId = capsuleBorderObjId;
    int i;
    int j;

    for (i = 0; i < 6; i++) {
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId == objId) {
                *borderId = -1;
            }
        }
    }
    for (i = 0; i < 6; i++) {
        borderId = &capsuleBorderObjId[i * 64];
        for (j = 0; j < 64; j++, borderId++) {
            if (*borderId >= 0) {
                break;
            }
        }
        if (j >= 64 && capsuleObjBorderId[i] >= 0) {
            mbObjKill(capsuleObjBorderId[i]);
            capsuleObjBorderId[i] = -1;
        }
    }
}

static inline void CapObjUnitScaleSet(CAPSULE_OBJ_COLOR *obj)
{
    obj->scale.x = obj->scale.y = obj->scale.z = 1.0f;
}

// Throwing, capsule events, and shops allocate a colored capsule model on render layer four.
// Packed owner bits are discarded; a full model-slot table returns -1.
int mbCapObjColorCreate(int capsuleNo, BOOL createF)
{
    CAPSULE_OBJ_COLOR *obj;
    int i;
    int capValue;
    u32 file;
    s16 initialValue;
    s16 capValueShort;
    int objFile;
    int fileNo;

    obj = capsuleObjColorData;
    initialValue = (s16)capsuleNo & CAPSULE_VALUE_TYPE_MASK;
    capsuleNo = initialValue;
    for (i = 0; i < CAPSULE_OBJ_COLOR_MAX; i++, obj++) {
        if (!obj->flag) {
            break;
        }
    }
    if (i >= CAPSULE_OBJ_COLOR_MAX) {
        return -1;
    }
    capValue = capsuleNo;
    capValueShort = (s16)capValue & CAPSULE_VALUE_TYPE_MASK;
    capValue = capValueShort;
    objFile = capsuleData[capValue].objFile;
    fileNo = objFile;
    file = fileNo;
    obj->flag = TRUE;
    obj->mdlId = mbObjCreate(file, NULL, createF);
    obj->layer = 4;
    obj->pos.x = obj->pos.y = obj->pos.z = 0.0f;
    obj->rot.x = obj->rot.y = obj->rot.z = 0.0f;
    CapObjUnitScaleSet(obj);
    mbCapObjColorLayerSet(obj->mdlId, obj->layer);
    return obj->mdlId;
}

// Colored capsule accessors find the occupied slot for a board model ID, or return -1.
static inline int CapObjColorSearch(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int i;

    for (obj = capsuleObjColorData, i = 0; i < CAPSULE_OBJ_COLOR_MAX; i++, obj++) {
        if (obj->flag && obj->mdlId == id) {
            break;
        }
    }
    if (i < CAPSULE_OBJ_COLOR_MAX) {
        return i;
    } else {
        return -1;
    }
}

// Throwing, capsule events, and shop cleanup release a tracked colored capsule model.
void mbCapObjColorKill(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        if (obj->flag) {
            mbObjKill(obj->mdlId);
            obj->mdlId = MB_MODEL_NONE;
            obj->unusedModelId = MB_MODEL_NONE;
            obj->flag = FALSE;
        }
    }
}

// Capsule animations store and apply a colored model's base position in board world units.
void mbCapObjColorPosSet(int id, float posX, float posY, float posZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->pos.x = posX;
        obj->pos.y = posY;
        obj->pos.z = posZ;
        mbObjPosSetV(obj->mdlId, &obj->pos);
    }
}

// Capsule animations store and apply a colored model's three rotation angles in degrees.
void mbCapObjColorRotSet(int id, float rotX, float rotY, float rotZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->rot.x = rotX;
        obj->rot.y = rotY;
        obj->rot.z = rotZ;
        mbObjRotSetV(obj->mdlId, &obj->rot);
    }
}

// Capsule animations store and apply a colored model's per-axis scale.
void mbCapObjColorScaleSet(int id, float scaleX, float scaleY, float scaleZ)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->scale.x = scaleX;
        obj->scale.y = scaleY;
        obj->scale.z = scaleZ;
        mbObjScaleSetV(obj->mdlId, &obj->scale);
    }
}

// Flight trails and capsule animations set a colored model's base position from a vector.
void mbCapObjColorPosSetV(int id, HuVecF *pos)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorPosSet(id, pos->x, pos->y, pos->z);
    }
}

// Capsule animations set a colored model's stored rotation from a vector of degree angles.
void mbCapObjColorRotSetV(int id, HuVecF *rot)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorRotSet(id, rot->x, rot->y, rot->z);
    }
}

// Capsule animations set a colored model's stored per-axis scale from a vector.
void mbCapObjColorScaleSetV(int id, HuVecF *scale)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbCapObjColorScaleSet(id, scale->x, scale->y, scale->z);
    }
}

// Capsule animation callers read the colored model's stored base position; unknown IDs do nothing.
void mbCapObjColorPosGet(int id, HuVecF *pos)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *pos = obj->pos;
    }
}

// Capsule animation callers read the stored rotation in degrees; unknown IDs leave rot unchanged.
void mbCapObjColorRotGet(int id, HuVecF *rot)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *rot = obj->rot;
    }
}

// Capsule animation callers read the stored per-axis scale; unknown IDs leave scale unchanged.
void mbCapObjColorScaleGet(int id, HuVecF *scale)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        *scale = obj->scale;
    }
}

// Colored model creation and capsule animations store and apply the requested render layer.
void mbCapObjColorLayerSet(int id, u8 layer)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        obj->layer = layer;
        mbObjLayerSet(obj->mdlId, obj->layer);
    }
}

// Colored capsule callers read the last assigned render layer, or zero for an unknown model ID.
u8 mbCapObjColorLayerGet(int id)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) == -1) {
        return 0;
    }
    obj = &capsuleObjColorData[idx];
    return obj->layer;
}

// Capsule animations enable supplied engine model or motion attribute bits on a colored model.
void mbCapObjColorAttrSet(int id, u32 attr)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;
    MBMODELID mdlId;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mdlId = obj->mdlId;
        mbObjAttrSet(mdlId, attr);
    }
}

// Capsule animations clear supplied engine model or motion attribute bits on a colored model.
void mbCapObjColorAttrReset(int id, u32 attr)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;
    MBMODELID mdlId;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mdlId = obj->mdlId;
        mbObjAttrReset(mdlId, attr);
    }
}

// Capsule animations request colored model visibility for the next board object-manager update.
void mbCapObjColorDispSet(int id, BOOL dispF)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjDispSet(obj->mdlId, dispF);
    }
}

// Capsule animations set colored model opacity from zero (transparent) to 255 (opaque).
void mbCapObjColorAlphaSet(int id, u8 alpha)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjAlphaSet(obj->mdlId, alpha);
    }
}

// Capsule animations replace a colored model's engine transformation matrix directly.
void mbCapObjColorMtxSet(int id, Mtx *mtx)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjMtxSet(obj->mdlId, mtx);
    }
}

// Capsule animation callers copy a colored model's current engine transformation matrix.
void mbCapObjColorMtxGet(int id, Mtx *mtx)
{
    CAPSULE_OBJ_COLOR *obj;
    int idx;

    if ((idx = CapObjColorSearch(id)) != -1) {
        obj = &capsuleObjColorData[idx];
        mbObjMtxGet(obj->mdlId, mtx);
    }
}

// Creates ten hidden route-guide models when capsule destination selection starts.
static void CapGuideCreate(void)
{
    CAP_GUIDE_WORK *work;
    CAP_GUIDE_WORK *workBase;
    int i;

    workBase = HuMemDirectMallocNum(HEAP_HEAP, sizeof(CAP_GUIDE_WORK) * 10, HU_MEMNUM_OVL);
    work = workBase;
    memset(work, 0, sizeof(CAP_GUIDE_WORK) * 10);
    capsuleGuideOMObj = omAddObjEx(mbObjMan, -32768, 0, 0, -1, CapGuideOMExec);
    capsuleGuideOMObj->work[0] = FALSE;
    capsuleGuideOMObj->data = work;
    for (i = 0; i < 10; i++, work++) {
        work->objId = mbObjCreate(DATANUM(DATA_capsule, 35), NULL, TRUE);
        mbObjLayerSet(work->objId, 5);
        mbObjDispSet(work->objId, FALSE);
        work->state = 0;
        work->rotY = 0;
        work->scale = 0.0f;
    }
}

// Human destination selection calls this to place a guide toward a linked space.
// Uses the first hidden guide and delays its animation by one object update.
static void CapGuideRotYSet(int masuId, float rotY)
{
    CAP_GUIDE_WORK *work = capsuleGuideOMObj->data;
    HuVecF masuPos;
    HuVecF pos;
    int zOfs;
    int i;

    if (capsuleGuideOMObj != NULL) {
        mbMasuPosGet(masuId, &masuPos);
        for (i = 0; i < 10; i++, work++) {
            if (mbObjDispGet(work->objId) == TRUE) {
                continue;
            }
            {
                s16 objId;

                zOfs = 0;

                mbObjRotSet(work->objId, 0.0f, rotY, 0.0f);
                pos.x = masuPos.x + 150.0f * HuSin(rotY);
                pos.y = masuPos.y + 10.0f;
                pos.z = masuPos.z + 150.0f * HuCos(rotY) + zOfs;
                mbObjPosSetV(work->objId, &pos);
                mbObjScaleSet(work->objId, 0.0f, 0.0f, 0.0f);
                mbObjDispSet(work->objId, TRUE);
                mbObjLayerSet(work->objId, 5);
                objId = work->objId;
                mbObjAttrSet(objId, HU3D_ATTR_ZCMP_OFF);
                work->state = 0;
                work->rotY = 0;
                work->scale = 0.0f;
                break;
            }
        }
        capsuleGuideOMObj->work[0] = FALSE;
        capsuleGuideOMObj->work[1] = TRUE;
    }
}

// Object-manager update grows or shrinks route guides and advances their pulse phase.
// Releases all guide models after selection ends or the board exits.
static void CapGuideOMExec(OMOBJ *obj)
{
    CAP_GUIDE_WORK *work = obj->data;
    int i;
    int rotY;

    if (mbExitCheck() || capsuleGuideOMObj == NULL) {
        for (i = 0; i < 10; i++, work++) {
            mbObjKill(work->objId);
        }
        omDelObjEx(mbObjMan, obj);
        capsuleGuideOMObj = NULL;
        return;
    }
    if (capsuleGuideOMObj->work[1] != 0) {
        capsuleGuideOMObj->work[1]--;
        return;
    }
    for (i = 0; i < 10; i++, work++) {
        if (!mbObjDispGet(work->objId)) {
            continue;
        }
        if (capsuleGuideOMObj->work[0] == 0) {
            switch (work->state) {
            case 0:
                if (work->scale < 2.0f) {
            work->scale += 0.2;
                } else {
                    work->scale = 2.0f;
                    work->state = 1;
                }
                mbObjScaleSet(work->objId, work->scale, work->scale, work->scale);
            // Growth falls through; the pulse scale is stored without a model update.
            case 1:
                if ((work->rotY += 20) > 360) {
                    work->rotY -= 360;
                }
                rotY = work->rotY;
            work->scale = (float)(1.0 +
                (0.2f * sin((M_PI * rotY) / 180.0)));
                break;
            }
        } else {
            if ((work->scale -= 0.2f) < 0.0f) {
                work->state = 0;
                mbObjDispSet(work->objId, FALSE);
            }
            mbObjScaleSet(work->objId, work->scale, work->scale, work->scale);
        }
    }
}

// Requests route-guide shrink when human selection moves to another space.
static void CapGuideGrowSet(void)
{
    void *work = capsuleGuideOMObj->data;

    if (capsuleGuideOMObj != NULL) {
        capsuleGuideOMObj->work[0] = TRUE;
    }
}

// Requests deferred guide deletion during destination-selection cleanup.
static void CapGuideKill(void)
{
    capsuleGuideOMObj = NULL;
}

// Creates 32 eligible-space markers and one animated current-space marker for selection.
static void CapEffMasuOkCreate(void)
{
    OMOBJ *obj;
    CAP_EFF_MASU_OK_WORK *work;
    int i;

    obj = capEffMasuOkOMObj = omAddObjEx(
        mbObjMan, -32768, 0, 0, -1, CapEffMasuOkOMExec);
    work = obj->data = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*work) * 33, HU_MEMNUM_OVL);
    memset(work, 0, sizeof(*work) * 33);
    for (i = 0; i < 32; i++, work++) {
        work->modelId = mbObjCreate(DATANUM(DATA_capsule, 36), NULL, TRUE);
        mbObjDispSet(work->modelId, FALSE);
        {
            MBMODELID modelId = work->modelId;

            mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
        }
        mbObjLayerSet(work->modelId, 5);
        work->masuId = 0;
        work->state = 0;
        work->unusedWord = 0;
        work->scale = 0.0f;
    }
    work->modelId = mbObjCreate(DATANUM(DATA_capsule, 37), NULL, TRUE);
    mbObjDispSet(work->modelId, FALSE);
    {
        MBMODELID modelId = work->modelId;

        mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    }
    mbObjLayerSet(work->modelId, 5);
    work->masuId = 0;
    work->state = 0;
    work->unusedWord = 0;
    work->scale = 0.0f;
}

// Human selection assigns the eligible-space markers from its destination table.
// The starting-space argument is unused.
static void CapEffMasuOkAddAll(s16 unused, s16 *masuFlag)
{
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;
    HuVecF pos;
    HuVecF rot;
    int masuId;
    int count;

    for (masuId = 1, count = 0; masuId <= mbMasuNumGet(); masuId++) {
        if (masuFlag[masuId] & 1) {
            mbMasuPosGet(masuId, &pos);
            mbMasuRotGet(masuId, &rot);
            pos.y += 10.0f;
            mbObjPosSetV(work->modelId, &pos);
            mbObjRotSetV(work->modelId, &rot);
            work->masuId = masuId;
            work->state = 1;
            count++;
            work++;
        }
    }
}

// Object-manager update follows marked spaces and animates the current-space marker.
// Releases the 33 models after selection ends or the board exits.
static void CapEffMasuOkOMExec(OMOBJ *obj)
{
    CAP_EFF_MASU_OK_WORK *work = obj->data;
    HuVecF pos;
    HuVecF rot;
    int i;

    if (mbExitCheck() || capEffMasuOkOMObj == NULL) {
        for (i = 0; i < 32; i++, work++) {
            mbObjKill(work->modelId);
        }
        mbObjKill(work->modelId);
        omDelObjEx(mbObjMan, obj);
        capEffMasuOkOMObj = NULL;
        return;
    }
    for (i = 0; i < 32; i++, work++) {
        if (work->masuId > 0) {
            mbMasuPosGet(work->masuId, &pos);
            mbMasuRotGet(work->masuId, &rot);
            pos.y += 10.0f;
            mbObjPosSetV(work->modelId, &pos);
            mbObjRotSetV(work->modelId, &rot);
        }
    }
    if (work->masuId > 0) {
        mbMasuPosGet(work->masuId, &pos);
        mbMasuRotGet(work->masuId, &rot);
        pos.y += 10.0f;
        mbObjPosSetV(work->modelId, &pos);
        mbObjRotSetV(work->modelId, &rot);
    }
    switch (work->state) {
        case 1:
            work->state++;
            break;

        case 2:
            work->scale += 0.2f;
            if (work->scale >= 1.0f) {
                work->scale = 1.0f;
                work->state++;
            }
            mbObjScaleSet(work->modelId, work->scale, work->scale, work->scale);
            break;

        case 10:
            work->scale -= 0.2f;
            if (work->scale <= 0.001f) {
                mbObjDispSet(work->modelId, FALSE);
                work->scale = 0.001f;
                work->state++;
            }
            mbObjScaleSet(work->modelId, work->scale, work->scale, work->scale);
            break;
    }
}

// Human selection shows or hides eligible-space markers; the current marker is separate.
static void CapEffMasuOkDispSet(BOOL dispF)
{
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;
    int i;

    for (i = 0; i < 32; i++, work++) {
        if (work->state == 1 && dispF) {
            mbObjDispSet(work->modelId, TRUE);
        } else {
            mbObjDispSet(work->modelId, FALSE);
        }
    }
}

// Requests deferred marker deletion during destination-selection cleanup.
static void CapEffMasuOkKill(void)
{
    capEffMasuOkOMObj = NULL;
}

// Human selection places the current-space marker ten units above a valid destination.
// Starts its growth animation from a nearly invisible scale.
static void CapEffMasuOkPosSet(HuVecF *pos, int masuId)
{
    HuVecF markerPos;
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;

    work += 32;
    markerPos = *pos;
    markerPos.y += 10.0f;
    mbObjPosSetV(work->modelId, &markerPos);
    mbObjDispSet(work->modelId, TRUE);
    mbObjScaleSet(work->modelId, 0.001f, 0.001f, 0.001f);
    work->masuId = masuId;
    work->state = 1;
    work->scale = 0.0f;
}

// Starts the current-space marker shrinking when selection moves to another space.
static void CapEffMasuOkNext(void)
{
    OMOBJ *obj = capEffMasuOkOMObj;
    CAP_EFF_MASU_OK_WORK *work = obj->data;

    work += 32;
    work->state = 10;
}

// Capsule discard starts a removal effect with space for sixty-four animated particles.
static void CapEffRemoveCreate(void)
{
    OMOBJ *obj;
    CAP_EFF_REMOVE_WORK *work;
    CAP_EFFECT *effect;
    HU3D_MODEL *model;
    ANIMDATA *anim;
    CAP_EFF_REMOVE_WORK *newWork;
    int modelId;

    obj = capEffRemoveOMObj = omAddObjEx(
        mbObjMan, -32768, 0, 0, OM_GRP_NONE, CapEffRemoveOMExec);
    newWork = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    obj->data = newWork;
    work = newWork;
    memset(work, 0, sizeof(*work));
    anim = HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, CAPSULE_DATA_REMOVE_ANIM), HU_MEMNUM_OVL));
    work->anim = anim;
    work->modelId = modelId = CapEffCreate(anim, 64);
    Hu3DModelLayerSet(work->modelId, 5);
    work->activeCount = 0;
    model = &Hu3DData[work->modelId];
    effect = model->hookData;
    effect->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
}

// Object-manager updates move removal particles until their sixteen-frame animation ends.
// Board exit or a cleanup request releases the model, texture animation, and object.
static void CapEffRemoveOMExec(OMOBJ *obj)
{
    CAP_EFF_REMOVE_WORK *work = obj->data;
    HU3D_MODEL *model;
    CAP_EFFECT *effect;
    CAP_EFF_DATA *data;
    int i;

    if (mbExitCheck() || capEffRemoveOMObj == NULL) {
        Hu3DModelKill(work->modelId);
        work->modelId = MB_MODEL_NONE;
        HuSprAnimKill(work->anim);
        work->anim = NULL;
        omDelObjEx(mbObjMan, obj);
        capEffRemoveOMObj = NULL;
        return;
    }
    if (work->activeCount <= 0) {
        Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        return;
    }
    Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
    model = &Hu3DData[work->modelId];
    effect = model->hookData;
    data = effect->data;
    effect->unusedByte = 0;
    for (i = 0; i < effect->num; i++, data++) {
        if (data->scale <= 0.0f) {
            continue;
        }
        data->pos.x += data->vel.x;
        data->pos.y += data->vel.y;
        data->pos.z += data->vel.z;
        data->rot.z += data->speed;
        if (data->rot.z >= 360.0f) {
            data->rot.z -= 360.0f;
        }
        data->frameNo = (int)data->animTime;
        data->animTime += data->animSpeed;
        if (data->frameNo >= 16) {
            data->frameNo = 0;
            data->phase = 0;
            data->scale = 0.0f;
            work->activeCount--;
        }
    }
}

// Capsule discard requests removal-effect cleanup on the next object-manager update.
static void CapEffRemoveKill(void)
{
    capEffRemoveOMObj = NULL;
}

// Capsule discard waits while the removal effect still has active particles.
static BOOL CapEffRemoveCheck(void)
{
    OMOBJ *obj = capEffRemoveOMObj;
    CAP_EFF_REMOVE_WORK *work;

    if (capEffRemoveOMObj == NULL) {
        return FALSE;
    }
    work = obj->data;
    return work->activeCount;
}

// Removal bursts fill the first inactive particle slot, returning -1 when the pool is full.
// The offset argument is unused here; the caller has already applied it to the position.
static inline int CapEffRemoveAddData(HuVecF pos, HuVecF vel, float scale,
    float speed, float offset, float animSpeed, GXColor color)
{
    CAP_EFF_DATA *data;
    CAP_EFFECT *effect;
    int i;
    CAP_EFF_REMOVE_WORK *work;
    OMOBJ *obj;
    HU3D_MODEL *model;

    obj = capEffRemoveOMObj;
    work = obj->data;
    model = &Hu3DData[work->modelId];
    effect = model->hookData;
    data = effect->data;
    for (i = 0; i < effect->num; i++, data++) {
        if (data->scale <= 0.0f) {
            break;
        }
    }
    if (i >= effect->num) {
        return -1;
    }
    data->phase = data->fadeFrame = 0;
    data->pos.x = pos.x;
    data->pos.y = pos.y;
    data->pos.z = pos.z;
    data->vel.x = vel.x;
    data->vel.y = vel.y;
    data->vel.z = vel.z;
    data->speed = speed;
    data->scale = scale;
    data->color = color;
    data->rot.z = 0.0f;
    data->frameNo = 0;
    data->phase = 0;
    data->animTime = 0.0f;
    data->animSpeed = animSpeed;
    work->activeCount++;
    return i;
}

// The discard burst adds two equally spaced particles with opposite rotation speeds.
// Returns the two slot results packed without a separate failure check.
static int CapEffRemoveAdd(HuVecF pos, HuVecF vel, float scale,
    float speed, float offset, float animSpeed, GXColor color)
{
    HuVecF finalPos;
    HuVecF dir;
    float dist;
    int firstIndex;
    int secondIndex;

    // Separation uses the swapped X/Z velocity components, without negating either one.
    dir.x = vel.z;
    dir.z = vel.x;
    dir.y = 0.0f;
    if (PSVECMag(&dir) > 0.0f) {
        PSVECNormalize(&dir, &dir);
    }
    dist = 0.5f * offset;

    finalPos.x = pos.x + (dir.x * dist);
    finalPos.y = pos.y + (dir.y * dist);
    finalPos.z = pos.z + (dir.z * dist);
    firstIndex = CapEffRemoveAddData(
        finalPos, vel, scale, speed, offset, animSpeed, color);

    finalPos.x = pos.x - (dir.x * dist);
    finalPos.y = pos.y - (dir.y * dist);
    finalPos.z = pos.z - (dir.z * dist);
    secondIndex = CapEffRemoveAddData(
        finalPos, vel, scale, -speed, offset, animSpeed, color);
    return (firstIndex << 16) | secondIndex;
}

// Confirming capsule discard emits thirty-two particle pairs around the hidden item model.
// Each pair gets a randomized radius, outward speed, size, tint, and animation speed.
static void CapEffRemoveAddAll(HuVecF *pos)
{
    HuVecF effectPos;
    HuVecF velocity;
    GXColor color;
    float angle;
    float dist;
    int i;

    for (i = 0; i < 32; i++) {
        angle = 11.25f * (float)i;
        dist = (100.0f * MBCapsuleEffRandF()) * 0.33f;
        effectPos.x = pos->x +
            (dist * cos((M_PI * angle) / 180.0));
        effectPos.y = pos->y +
            (dist * sin((M_PI * angle) / 180.0)) + 100.0f;
        effectPos.z = pos->z + 50.0f;

        dist = ((0.04f * MBCapsuleEffRandF()) + 0.005f)
            * 100.0f;
        velocity.x = dist * cos((M_PI * angle) / 180.0);
        velocity.y = dist * sin((M_PI * angle) / 180.0);
        velocity.z = 0.0f;

        dist = MBCapsuleEffRandF();
        color.r = (u8)(192.0f + (32.0f * dist));
        color.g = (u8)(192.0f + (32.0f * dist));
        color.b = (u8)(192.0f + (32.0f * dist));
        color.a = (u8)(192.0f + (63.0f * MBCapsuleEffRandF()));

        CapEffRemoveAdd(effectPos, velocity,
            100.0f * (1.0f + (0.5f * MBCapsuleEffRandF())),
            0.12f + (0.25f * MBCapsuleEffRandF()),
            100.0f * (0.5f + (0.5f * MBCapsuleEffRandF())),
            0.33f + (0.66f * MBCapsuleEffRandF()), color);
    }
}

// Capsule-use cleanup clears the auxiliary removal-effect object reference.
static void CapEffRemoveAddDestroy(void)
{
    capEffRemoveAddOMObj = NULL;
}

static int capsuleBorderFileTbl[6] = {
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_0),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_1),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_2),
    DATANUM(DATA_capsule, CAPSULE_DATA_BORDER_3),
    0,
    0,
};

static int capsuleHiliteFileTbl[3] = {
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_0),
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_1),
    DATANUM(DATA_capsule, CAPSULE_DATA_HILITE_2),
};

static HuVecF capsuleCrackScaleTbl[2][3] = {
    {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
    },
    {
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 1.0f },
    },
};

static GXColor capsuleTrailColorTbl[4][2] = {
    {
        { 128, 255, 128, 255 },
        { 192, 255, 192, 255 },
    },
    {
        { 255, 255, 128, 255 },
        { 255, 255, 192, 255 },
    },
    {
        { 255, 128, 128, 255 },
        { 255, 192, 192, 255 },
    },
    {
        { 128, 255, 255, 255 },
        { 192, 255, 255, 255 },
    },
};

// Human destination selection allocates three additive highlight textures with thirty-two slots
// each.
static void CapEffHiliteCreate(void)
{
    OMOBJ *obj;
    CAP_EFF_HILITE_WORK *work;
    HU3D_MODEL *model;
    int modelId;
    ANIMDATA *anim;
    CAP_EFF_HILITE_WORK *newWork;
    CAP_EFFECT *effect;
    int i;

    obj = capEffHiliteOMObj = omAddObjEx(
        mbObjMan, -32768, 0, 0, -1, CapEffHiliteOMExec);
    newWork = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    obj->data = newWork;
    work = newWork;
    memset(work, 0, sizeof(*work));
    for (i = 0; i < 3; i++) {
        anim = HuSprAnimRead(HuDataReadNum(
            capsuleHiliteFileTbl[i], HU_MEMNUM_OVL));
        work->anim[i] = anim;
        work->modelId[i] = modelId = CapEffCreate(anim, 32);
        Hu3DModelLayerSet(modelId, 5);
        work->activeCount = 0;
        model = &Hu3DData[modelId];
        effect = model->hookData;
        effect->blendMode = HU3D_PARTICLE_BLEND_ADDCOL;
        effect->dispAttr = CAP_EFF_DISPATTR_NOANIM |
            CAP_EFF_DISPATTR_CAMERA_ROT | CAP_EFF_DISPATTR_ROT3D;
    }
}

// Object-manager updates grow and fade destination highlights using their selected easing mode.
// All three texture models share one active count and are released when selection ends.
static void CapEffHiliteOMExec(OMOBJ *obj)
{
    CAP_EFF_HILITE_WORK *work;
    HU3D_MODEL *model;
    CAP_EFFECT *effect;
    CAP_EFF_DATA *data;
    float t;
    int modelNo;
    int i;

    work = obj->data;
    if (mbExitCheck() || capEffHiliteOMObj == NULL) {
        for (modelNo = 0; modelNo < 3; modelNo++) {
            Hu3DModelKill(work->modelId[modelNo]);
            work->modelId[modelNo] = MB_MODEL_NONE;
        }
        for (modelNo = 0; modelNo < 3; modelNo++) {
            HuSprAnimKill(work->anim[modelNo]);
            work->anim[modelNo] = NULL;
        }
        omDelObjEx(mbObjMan, obj);
        capEffHiliteOMObj = NULL;
        return;
    }
    for (modelNo = 0; modelNo < 3; modelNo++) {
        if (work->activeCount <= 0) {
            Hu3DModelAttrSet(work->modelId[modelNo], HU3D_ATTR_DISPOFF);
            continue;
        }
        Hu3DModelAttrReset(work->modelId[modelNo], HU3D_ATTR_DISPOFF);
        model = &Hu3DData[work->modelId[modelNo]];
        effect = model->hookData;
        data = effect->data;
        effect->unusedByte = 0;
        for (i = 0; i < effect->num; i++, data++) {
            if (data->scale <= 0.0f) {
                continue;
            }
            switch (data->phase) {
            case 0:
                if (data->fadeInDuration > 0.0f) {
                    t = (float)(++data->fadeFrame) / data->fadeInDuration;
                } else {
                    t = 1.0f;
                }
                switch (data->easingMode) {
                case 0:
                    (void)t;
                    break;
                case 1:
                    t = (float)sin((M_PI * (90.0f * t)) / 180.0);
                    break;
                case 2:
                    t = (float)cos(
                        (M_PI * (90.0f * (1.0f - t))) / 180.0);
                    break;
                }
                data->scale = data->vel.z *
                    (data->vel.x + (t * (1.0f - data->vel.x)));
                data->color.a = (u8)(data->speed * t);
                if (t >= 1.0f) {
                    data->scale = data->vel.z;
                    data->color.a = (u8)data->speed;
                    data->phase++;
                    data->fadeFrame = 0;
                }
                break;

            case 1:
                if (data->fadeOutDuration > 0.0f) {
                    t = (float)(++data->fadeFrame) / data->fadeOutDuration;
                } else {
                    t = 1.0f;
                }
                switch (data->easingMode) {
                case 0:
                    (void)t;
                    break;
                case 1:
                    t = (float)sin((M_PI * (90.0f * t)) / 180.0);
                    break;
                case 2:
                    t = (float)cos(
                        (M_PI * (90.0f * (1.0f - t))) / 180.0);
                    break;
                }
                data->scale = data->vel.z *
                    (1.0f + (t * (data->vel.y - 1.0f)));
                data->color.a = (u8)(data->speed * (1.0f - t));
                if (t >= 1.0f) {
                    data->scale = 0.0f;
                    work->activeCount--;
                }
                break;
            }
        }
    }
}

// Selection cleanup requests highlight deletion on the next object-manager update.
static void CapEffHiliteKill(void)
{
    capEffHiliteOMObj = NULL;
}

// Destination selection adds a highlight with update-count fade durations and easing mode.
// scale.x and scale.y are initial and final scale ratios; scale.z supplies its full size.
static int CapEffHiliteAdd(HuVecF pos, HuVecF rot, HuVecF scale,
    int fadeIn, int fadeOut, int modelNo, int mode, GXColor color)
{
    OMOBJ *obj;
    CAP_EFF_HILITE_WORK *work;
    HU3D_MODEL *model;
    CAP_EFFECT *effect;
    CAP_EFF_DATA *data;
    int i;

    if (capEffHiliteOMObj == NULL) {
        return MB_MODEL_NONE;
    }
    obj = capEffHiliteOMObj;
    work = obj->data;
    model = &Hu3DData[work->modelId[modelNo % 3]];
    effect = model->hookData;
    data = effect->data;
    for (i = 0; i < effect->num; i++, data++) {
        if (data->scale <= 0.0f) {
            break;
        }
    }
    if (i >= effect->num) {
        return MB_MODEL_NONE;
    }
    data->phase = data->fadeFrame = 0;
    data->pos.x = pos.x;
    data->pos.y = pos.y;
    data->pos.z = pos.z;
    data->vel.x = scale.x;
    data->vel.y = scale.y;
    data->vel.z = scale.z;
    data->fadeInDuration = (float)fadeIn;
    data->fadeOutDuration = (float)fadeOut;
    data->speed = (float)color.a;
    data->scale = scale.z;
    data->color = color;
    data->rot.x = rot.x;
    data->rot.y = rot.y;
    data->rot.z = rot.z;
    data->frameNo = 0;
    data->phase = 0;
    data->easingMode = (s16)mode;
    work->activeCount++;
    return i;
}

// Shell construction calculates positive lengths, storing the float result before returning it.
// Nonpositive values pass through unchanged and leave result untouched.
static inline float CapEffCrackSqrt(
    float value, volatile float *result)
{
    const double sqrtHalf = .5;
    const double sqrtThree = 3.0;
    if (value > 0.0f) {
        double sqrtGuess = __frsqrte((double)value);
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        *result = (float)(value * sqrtGuess);
        return *result;
    }
    return value;
}

// Shell construction calculates a positive length and copies the stored float to output.
// Nonpositive values are copied directly to output and leave result untouched.
static inline void CapEffCrackSqrtStore(
    float value, volatile float *result, volatile float *output)
{
    const double sqrtHalf = .5;
    const double sqrtThree = 3.0;
    if (value > 0.0f) {
        double sqrtGuess = __frsqrte((double)value);
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        sqrtGuess = sqrtHalf * sqrtGuess
            * (sqrtThree - (sqrtGuess * sqrtGuess * value));
        *result = (float)(value * sqrtGuess);
        *output = *result;
    } else {
        *output = value;
    }
}

// Throw setup builds a textured dome from a twenty-four-by-twenty-four grid of triangle pairs.
// The shell remains idle until the landing burst starts its expansion and breakup.
static void CapEffCrackCreate(void)
{
    CAP_EFF_CRACK_DATA *data;
    CAP_EFF_CRACK_WORK *work;
    int l;
    int i;
    HU3D_MODEL *model;
    int k;
    int j;
    void *dlBuf;
    OMOBJ *obj;
    void *dlBegin;
    int dataSize;
    u32 dataHeap;
    int vtxSize;
    u32 vtxHeap;
    int stSize;
    u32 stHeap;
    volatile float sqrtScaleResult;
    volatile float sqrtHeightResult;
    volatile float sqrtVelocityValue;
    volatile float sqrtVelocityResult;
    u32 dlBufHeap;
    u32 dlSizeData;
    u32 dlDataHeap;
    CAP_EFF_CRACK_WORK *workData;
    CAP_EFF_CRACK_DATA *dataBuf;
    CAP_EFF_CRACK_DATA *dataP;
    HuVecF *vtxBuf;
    HuVecF *vtxP;
    HuVec2f *stBuf;
    HuVec2f *stP;
    void *dlBufData;
    void *dlBeginData;
    void *dlData;
    void *dlP;
    float yOfs;
    float y;
    float ofsX;
    float ofsZ;
    float scale;
    float scaleX;
    float scaleZ;
    HuVecF vtxPos;
    float scaleTbl[3];

    obj = capEffCrackOMObj = omAddObjEx(mbObjMan, -32768, 0, 0,
        OM_GRP_NONE, CapEffCrackOMExec);
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*work), HU_MEMNUM_OVL);
    work = obj->data = workData;
    memset(work, 0, sizeof(*work));
    work->modelId = Hu3DHookFuncCreate(CapEffCrackDraw);
    Hu3DModelCameraSet(work->modelId, 1);
    Hu3DModelLayerSet(work->modelId, 3);
    model = &Hu3DData[work->modelId];
    model->hookData = work;
    work->state = 0;
    work->time = 0;
    work->num = 24 * 24 * 2;
    work->vtxNum = work->num * 3;
    dataHeap = model->mallocNo;
    dataSize = work->num * sizeof(*work->data);
    dataBuf = HuMemDirectMallocNum(HEAP_MODEL, dataSize, dataHeap);
    dataP = dataBuf;
    data = work->data = dataP;
    memset(data, 0, work->num * sizeof(*work->data));
    vtxHeap = model->mallocNo;
    vtxSize = work->vtxNum * sizeof(*work->vtx);
    vtxBuf = HuMemDirectMallocNum(HEAP_MODEL, vtxSize, vtxHeap);
    vtxP = vtxBuf;
    work->vtx = vtxP;
    memset(work->vtx, 0, work->vtxNum * sizeof(*work->vtx));
    stHeap = model->mallocNo;
    stSize = work->vtxNum * sizeof(*work->st);
    stBuf = HuMemDirectMallocNum(HEAP_MODEL, stSize, stHeap);
    stP = stBuf;
    work->st = stP;
    memset(work->st, 0, work->vtxNum * sizeof(*work->st));
    work->animP = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_capsule, 43),
        HU_MEMNUM_OVL));
    work->color.r = 0;
    work->color.g = 0;
    work->color.b = 0;
    work->color.a = 0;

    scaleX = 8.333334f;
    scaleZ = 8.333334f;
    ofsX = -100.0f;
    ofsZ = -100.0f;
    for (i = 0; i < 24; i++) {
        for (j = 0; j < 24; j++) {
            for (k = 0; k < 2; data++, k++) {
                data->flag = TRUE;
                data->scale = 1.0f;
                data->angle = 0.0f;
                if (mbRandMod(CAPSULE_EFF_COLOR_RANGE) & 1) {
                    data->angleSpeed = (2.0f * MBCapsuleEffRandF()) + 5.0f;
                } else {
                    data->angleSpeed = -((2.0f * MBCapsuleEffRandF()) + 5.0f);
                }
                data->scaleSpeed = 0.035f + (0.035f * MBCapsuleEffRandF());
                data->delay = mbRandMod(CAPSULE_EFF_COLOR_RANGE) & 15;
                for (l = 0; l < 3; l++) {
                    data->pos[l].x = ofsX + (scaleX * capsuleCrackScaleTbl[k][l].x);
                    data->pos[l].z = ofsZ + (scaleZ * capsuleCrackScaleTbl[k][l].z);
                    data->pos[l].y = 0.0f;
                    scale = CapEffCrackSqrt(
                        (data->pos[l].x * data->pos[l].x)
                            + (data->pos[l].z * data->pos[l].z),
                        &sqrtScaleResult) / 100.0f;
                    scaleTbl[l] = scale;
                    // Clamp the circular rim before lifting each point onto the dome.
                    if (scaleTbl[l] > 1.0f) {
                        scale = 1.0f;
                        if (PSVECMag(&data->pos[l]) > 0.0f) {
                            PSVECNormalize(&data->pos[l], &data->pos[l]);
                        }
                        PSVECScale(&data->pos[l], &data->pos[l], 100.0f);
                    }
                    yOfs = 100.0f;
                    y = 100.0f * scale;
                    data->pos[l].y = CapEffCrackSqrt(
                        (yOfs * yOfs) - (y * y), &sqrtHeightResult);
                    yOfs = 100.0f;
                    data->uv[l].x = (data->pos[l].x + yOfs) / 200.0f;
                    data->uv[l].y = (data->pos[l].z + yOfs) / 200.0f;
                }
                vtxPos = data->pos[0];
                PSVECAdd(&vtxPos, &data->pos[1], &vtxPos);
                PSVECAdd(&vtxPos, &data->pos[2], &vtxPos);
                PSVECScale(&vtxPos, &data->vel, 0.333333f);
                CapEffCrackSqrtStore(
                    (data->vel.x * data->vel.x)
                        + (data->vel.z * data->vel.z),
                    &sqrtVelocityResult, &sqrtVelocityValue);
                yOfs = sqrtVelocityValue / 100;
                // This second delay draw replaces the earlier sixteen-step delay.
                data->delay = mbRandMod(CAPSULE_EFF_COLOR_RANGE) & 7;
                if (PSVECMag(&data->vel) > 0.0f) {
                    PSVECNormalize(&data->vel, &data->accel);
                }
                PSVECScale(&data->accel, &data->accel,
                    (0.5f + (0.5f * MBCapsuleEffRandF())) * 10.0f);
                data->prevVel = data->vel;
                for (l = 0; l < 3; l++) {
                    PSVECSubtract(&data->pos[l], &data->vel, &data->pos[l]);
                    data->prevPos[l] = data->pos[l];
                }
                data->color = capsuleCrackEffColor;
                if (scaleTbl[0] > 1.0f && scaleTbl[1] > 1.0f
                    && scaleTbl[2] > 1.0f) {
                    data->flag = FALSE;
                    data->scale = 0.0f;
                }
            }
            ofsX += scaleX;
        }
        ofsX = -100.0f;
        ofsZ += scaleZ;
    }

    dlBufHeap = model->mallocNo;
    dlBufData = HuMemDirectMallocNum(HEAP_MODEL, 65536, dlBufHeap);
    dlBeginData = dlBufData;
    dlBegin = dlBuf = dlBeginData;
    DCFlushRange(dlBuf, 65536);
    GXBeginDisplayList(dlBegin, 65536);
    GXBegin(GX_TRIANGLES, GX_VTXFMT0, work->vtxNum);
    for (i = 0; i < work->vtxNum / 3; i++) {
        GXPosition1x16(3 * i);
        GXColor1x16(i);
        GXTexCoord1x16(3 * i);
        GXPosition1x16((3 * i) + 1);
        GXColor1x16(i);
        GXTexCoord1x16((3 * i) + 1);
        GXPosition1x16((3 * i) + 2);
        GXColor1x16(i);
        GXTexCoord1x16((3 * i) + 2);
    }
    GXEnd();
    work->dlSize = GXEndDisplayList();
    // The size comparison has no effect; the display list is copied at its returned size.
    work->dlSize > 65536;
    dlDataHeap = model->mallocNo;
    dlSizeData = work->dlSize;
    dlData = HuMemDirectMallocNum(HEAP_MODEL, dlSizeData, dlDataHeap);
    dlP = dlData;
    work->dl = dlP;
    memcpy(work->dl, dlBuf, work->dlSize);
    DCFlushRange(work->dl, work->dlSize);
    HuMemDirectFree(dlBuf);
}

// Object-manager updates flatten, expand, raise, and disperse the landing burst's shell triangles.
// Each triangle waits its own delay, then moves outward, spins, shrinks, and fades away.
static void CapEffCrackOMExec(OMOBJ *obj)
{
    CAP_EFF_CRACK_WORK *work;
    CAP_EFF_CRACK_DATA *data;
    float scale;
    int i;
    int no;

    no = -1;
    work = obj->data;

    if (mbExitCheck() || capEffCrackOMObj == NULL) {
        HuSprAnimKill(work->animP);
        Hu3DModelKill(work->modelId);
        omDelObjEx(mbObjMan, obj);
        capEffCrackOMObj = NULL;
        return;
    }
    data = work->data;
    switch (work->state) {
        case 0:
            break;

        case 1:
            for (i = 0, data = work->data; i < work->num; i++, data++) {
                data->color.a = 64;
                data->vel.y = 0.0f;
                data->pos[0].y = 0.0f;
                data->pos[1].y = 0.0f;
                data->pos[2].y = 0.0f;
            }
            Hu3DModelScaleSet(work->modelId, 0.0f, 1.0f, 0.0f);
            work->time = 0;
            work->state++;
            break;

        case 2:
            scale = (float)++work->time / 6.0f;
            if (scale >= 1.0f) {
                scale = 1.0f;
            }
            Hu3DModelScaleSet(work->modelId, scale, 1.0f, scale);
            if ((float)work->time >= 6.0f) {
                work->time = 0;
                work->state++;
            }
            break;

        case 3:
            scale = (1.0f / 18.0f) * (float)++work->time;
            if (scale >= 1.0f) {
                scale = 1.0f;
            }
            for (i = 0, data = work->data; i < work->num; i++, data++) {
                data->color.a = (u8)(64.0f + (191.0f * scale));
                data->vel.y = data->prevVel.y * scale;
                data->pos[0].y = data->prevPos[0].y * scale;
                data->pos[1].y = data->prevPos[1].y * scale;
                data->pos[2].y = data->prevPos[2].y * scale;
            }
            work->color.r = work->color.g = work->color.b =
                (u8)(64.0f * scale);
            work->color.a = (u8)(255.0f * scale);
            if ((float)work->time >= 18.0f) {
                work->time = 0;
                work->state++;
            }
            break;

        case 4:
            if (work->color.a > 20) {
                work->color.a -= 20;
            } else {
                work->color.a = 0;
            }
            for (i = 0, data = work->data; i < work->num; i++, data++) {
                if (!data->flag) {
                    continue;
                }
                if (data->delay != 0) {
                    data->delay--;
                    continue;
                }
                PSVECAdd(&data->vel, &data->accel, &data->vel);
                data->angle += data->angleSpeed;
                if ((data->scale -= data->scaleSpeed) <= 0.0f) {
                    data->flag = FALSE;
                    continue;
                }
                if (data->scale > 1.0f) {
                    data->color.a = 255;
                } else {
                    data->color.a = (u8)(255.0f * data->scale);
                }
            }
            break;

        case 5:
            break;
    }
}

// Throw cleanup requests shell-effect deletion on the next object-manager update.
static void CapEffCrackKill(void)
{
    capEffCrackOMObj = NULL;
}

// The capsule landing burst starts shell expansion at the supplied position and degree rotation.
// Either missing vector defaults to zero; an absent shell object makes the request a no-op.
static void CapEffCrackAdd(HuVecF *pos, HuVecF *rot)
{
    OMOBJ *obj;
    CAP_EFF_CRACK_WORK *work;

    obj = capEffCrackOMObj;
    if (capEffCrackOMObj == NULL) {
        return;
    }
    work = obj->data;
    work->state = 1;
    if (pos != NULL) {
        obj->trans.x = pos->x;
        obj->trans.y = pos->y;
        obj->trans.z = pos->z;
    } else {
        obj->trans.x = obj->trans.y = obj->trans.z = 0.0f;
    }
    if (rot != NULL) {
        obj->rot.x = rot->x;
        obj->rot.y = rot->y;
        obj->rot.z = rot->z;
    } else {
        obj->rot.x = obj->rot.y = obj->rot.z = 0.0f;
    }
    Hu3DModelPosSet(
        work->modelId, obj->trans.x, obj->trans.y, obj->trans.z);
    Hu3DModelRotSet(work->modelId, obj->rot.x, obj->rot.y, obj->rot.z);
}

// The shell model's render hook rebuilds triangle vertices and draws them with additive blending.
// Idle shells and shadow passes are skipped; inactive triangles collapse to zero-sized faces.
static void CapEffCrackDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFF_CRACK_WORK *work;
    CAP_EFF_CRACK_DATA *data;
    int j;
    HuVecF *vtx;
    HuVec2f *st;
    int i;
    float sin;
    float cos;
    float sinAngle;
    float cosAngle;
    float sinResult;
    float sinValue;
    float cosResult;
    float cosValue;
    HuVecF pos;

    work = modelP->hookData;
    data = work->data;
    if (work->state == 0) {
        return;
    }
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
        GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    if (shadowModelDrawF) {
        return;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetTevColor(GX_TEVREG2, work->color);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_C2);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_A2);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
        GX_TRUE, GX_TEVPREV);
    GXSetChanAmbColor(GX_COLOR0A0, capsuleCrackEffMatColor);
    GXSetChanMatColor(GX_COLOR0A0, capsuleCrackEffAmbColor);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
        GX_DF_CLAMP, GX_AF_NONE);
    HuSprTexLoad(work->animP, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetZCompLoc(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_BACK);

    for (vtx = work->vtx, st = work->st, i = 0;
        i < work->num;
         i++, data++) {
        if (data->flag) {
            if (!data->angle) {
                for (j = 0; j < 3; j++) {
                    PSVECScale(&data->pos[j], &pos, data->scale);
                    PSVECAdd(&pos, &data->vel, vtx);
                    vtx++;
                    *st = data->uv[j];
                    st++;
                }
            } else {
                // Rotating triangles use their full vertex offsets rather than the shrinking scale.
                sinAngle = data->angle;
                sinResult = mbSinDeg(sinAngle);
                sinValue = sinResult;
                sin = sinValue;
                cosAngle = data->angle;
                cosResult = mbCosDeg(cosAngle);
                cosValue = cosResult;
                cos = cosValue;
                for (j = 0; j < 3; j++) {
                    PSVECScale(&data->pos[j], &pos, data->scale);
                    vtx->x = (data->vel.x + (data->pos[j].x * cos)) -
                        (data->pos[j].y * sin);
                    vtx->y = (data->vel.y + (data->pos[j].x * sin)) +
                        (data->pos[j].y * cos);
                    vtx->z = data->vel.z + data->pos[j].z;
                    vtx++;
                    *st = data->uv[j];
                    st++;
                }
            }
        } else {
            vtx->x = vtx->y = vtx->z = 0.0f;
            vtx++;
            vtx->x = vtx->y = vtx->z = 0.0f;
            vtx++;
            vtx->x = vtx->y = vtx->z = 0.0f;
            vtx++;
            st += 3;
        }
    }
    data = work->data;
    DCFlushRangeNoSync(work->vtx, work->vtxNum * sizeof(HuVecF));
    DCFlushRangeNoSync(work->st, work->vtxNum * sizeof(HuVec2f));
    DCFlushRangeNoSync(work->data, work->num * sizeof(CAP_EFF_CRACK_DATA));
    PPCSync();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetArray(GX_VA_POS, work->vtx, sizeof(*work->vtx));
    GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetArray(GX_VA_CLR0, &work->data->color, sizeof(*work->data));
    GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetArray(GX_VA_TEX0, work->st, sizeof(*work->st));
    GXCallDisplayList(work->dl, work->dlSize);
}

// Throw setup allocates a twelve-position flight history and two twelve-particle trail layers.
// The capsuleNo argument is masked but otherwise unused; particles stay hidden until flight starts.
static void CapEffTrailCreate(int capsuleNo)
{
    OMOBJ *obj;
    int i;
    MBPARTICLEDATA *particleData;
    CAP_EFF_TRAIL_WORK *work;
    s16 value;
    CAP_EFF_TRAIL_WORK *workData;
    MBPARTICLE *particle;
    HU3D_MODEL *model;

    value = (s16)capsuleNo & CAPSULE_VALUE_TYPE_MASK;
    capsuleNo = value;
    obj = capEffTrailOMObj = omAddObjEx(mbObjMan, -32768, 13, 0,
        OM_GRP_NONE,
        CapEffTrailOMExec);
    workData = HuMemDirectMallocNum(
        HEAP_HEAP, sizeof(*workData), HU_MEMNUM_OVL);
    work = obj->data = workData;
    memset(work, 0, sizeof(*work));
    for (i = 0; i < 12; i++) {
        obj->mdlId[i] = MB_MODEL_NONE;
        work->prevPos[i].x = work->prevPos[i].y = work->prevPos[i].z = 0.0f;
    }
    obj->mdlId[12] = mbParticleCreate(HuSprAnimRead(HuDataReadNum(
        DATANUM(DATA_capsule, 42), HU_MEMNUM_OVL)), 24);
    Hu3DModelCameraSet(obj->mdlId[12], 1);
    Hu3DModelLayerSet(obj->mdlId[12], 5);
    model = &Hu3DData[obj->mdlId[12]];
    particle = model->hookData;
    particleData = particle->data;
    particle->blendMode = MB_PARTICLE_BLEND_ADDCOL;
    for (i = 0; i < particle->num; i++, particleData++) {
        particleData->scale = 0.0f;
        particleData->pos.x = particleData->pos.y = particleData->pos.z = 0.0f;
    }
    obj->stat |= OM_STAT_MODELPAUSE;
    obj->work[0] = 0;
    obj->work[1] = 0;
}

// Object-manager updates record the capsule's flight position and redistribute its trail behind it.
// Samples are evenly spaced over short histories and ten world units apart beyond 120 units.
static void CapEffTrailOMExec(OMOBJ *obj)
{
    int i;
    CAP_EFF_TRAIL_POINT *trailPoint;
    MBPARTICLEDATA *particleData;
    CAP_EFF_TRAIL_WORK *work;
    int pointNo;
    MBPARTICLE *particle;
    int trailPointNo;
    float totalMag;
    float nextDist;
    float scale;
    CAP_EFF_TRAIL_POINT trailData[12];
    HuVecF pointHis[12];
    HuVecF dir;
    HU3D_MODEL *model;
    int j;
    int nextTrailPoint;

    work = obj->data;
    if (mbExitCheck() || capEffTrailOMObj == NULL) {
        for (i = 0; i < 12; i++) {
            if (obj->mdlId[i] != MB_MODEL_NONE) {
                mbCapObjColorKill(obj->mdlId[i]);
            }
            obj->mdlId[i] = MB_MODEL_NONE;
        }
        mbParticleKill(obj->mdlId[12]);
        obj->mdlId[12] = MB_MODEL_NONE;
        omDelObjEx(mbObjMan, obj);
        capEffTrailOMObj = NULL;
        return;
    }
    if (!obj->work[0]) {
        return;
    }
    pointNo = obj->work[1];
    work->prevPos[pointNo].x = obj->trans.x;
    work->prevPos[pointNo].y = obj->trans.y;
    work->prevPos[pointNo].z = obj->trans.z;
    for (trailPoint = trailData, totalMag = 0.0f, i = 0;
         i < 11;
         i++, trailPoint++) {
        if ((trailPointNo = i + obj->work[1]) >= 12) {
            trailPointNo -= 12;
        }
        if ((nextTrailPoint = trailPointNo + 1) >= 12) {
            nextTrailPoint -= 12;
        }
        PSVECSubtract(&work->prevPos[trailPointNo],
            &work->prevPos[nextTrailPoint], &dir);
        trailPoint->start = work->prevPos[trailPointNo];
        trailPoint->end = work->prevPos[nextTrailPoint];
        trailPoint->mag = PSVECMag(&dir);
        trailPoint->totalMag = totalMag;
        totalMag += trailPoint->mag;
    }
    for (i = 0; i < 12; i++) {
        if (totalMag <= 0.0f) {
            nextDist = 0.0f;
        } else if (totalMag <= 120.0f) {
            nextDist = (totalMag / 12.0f) * (float)i;
        } else {
            nextDist = 10.0f * (float)i;
        }
        for (j = 0, trailPoint = trailData;
             j < 11;
             j++, trailPoint++) {
            if (nextDist < trailPoint->totalMag + trailPoint->mag) {
                break;
            }
        }
        if (j < 11) {
            nextDist -= trailPoint->totalMag;
            if (trailPoint->mag > 0.0f) {
                scale = nextDist / trailPoint->mag;
                PSVECSubtract(&trailPoint->end, &trailPoint->start, &dir);
                PSVECScale(&dir, &dir, scale);
                PSVECAdd(&trailPoint->start, &dir, &pointHis[i]);
            } else {
                pointHis[i] = trailPoint->start;
            }
        } else {
            pointHis[i] = trailData[10].end;
        }
    }
    for (i = 0; i < 12; i++) {
        if (obj->mdlId[i] != MB_MODEL_NONE) {
            mbCapObjColorPosSetV(obj->mdlId[i], &pointHis[i]);
        }
    }
    model = &Hu3DData[obj->mdlId[12]];
    particle = model->hookData;
    particleData = particle->data;
    work = obj->data;
    for (i = 0; i < particle->num / 2; i++, particleData++) {
        particleData->pos.x = pointHis[i].x;
        particleData->pos.y = pointHis[i].y;
        particleData->pos.z = pointHis[i].z;
    }
    for (i = 0; i < particle->num / 2; i++, particleData++) {
        particleData->pos.x = pointHis[i].x;
        particleData->pos.y = pointHis[i].y;
        particleData->pos.z = pointHis[i].z;
    }
    pointNo--;
    if (pointNo < 0) {
        pointNo = 11;
    }
    obj->work[1] = pointNo;
}

// Throw cleanup requests flight-trail deletion on the next object-manager update.
static void CapEffTrailKill(void)
{
    capEffTrailOMObj = NULL;
}

// Flight start seeds the trail at one position and tints both layers to the capsule's color.
// Each layer becomes smaller and more transparent toward the trail's tail.
static void CapEffTrailAdd(HuVecF *pos, int capsuleNo)
{
    OMOBJ *obj = capEffTrailOMObj;
    CAP_EFF_TRAIL_WORK *work;
    HU3D_MODEL *model;
    MBPARTICLE *particle;
    MBPARTICLEDATA *data;
    GXColor color;
    float time;
    int i;

    if (capEffTrailOMObj == NULL) {
        return;
    }
    work = obj->data;
    for (i = 0; i < 12; i++) {
        work->prevPos[i] = *pos;
    }
    model = &Hu3DData[obj->mdlId[12]];
    particle = model->hookData;
    data = particle->data;
    work = obj->data;
    color = capsuleTrailColorTbl[mbCapColorGet(capsuleNo)][0];
    for (i = 0; i < particle->num / 2; i++, data++) {
        time = (float)i / 12.0f;
        data->scale = 100.0f *
            (0.9f + ((0.4f - 0.9f) * time));
        data->pos.x = pos->x;
        data->pos.y = pos->y;
        data->pos.z = pos->z;
        data->color.r = color.r;
        data->color.g = color.g;
        data->color.b = color.b;
        data->color.a = (u8)(32.0f * (1.0f - time));
    }
    color = capsuleTrailColorTbl[mbCapColorGet(capsuleNo)][1];
    for (i = 0; i < particle->num / 2; i++, data++) {
        time = (float)i / 12.0f;
        data->scale = 0.75f * (100.0f *
            (0.9f + ((0.4f - 0.9f) * time)));
        data->pos.x = pos->x;
        data->pos.y = pos->y;
        data->pos.z = pos->z;
        data->color.r = color.r;
        data->color.g = color.g;
        data->color.b = color.b;
        data->color.a = (u8)(64.0f * (1.0f - time));
    }
    obj->work[0] = 1;
}

// Flight animation supplies the position that the trail records on its next object update.
static void CapEffTrailPosSet(HuVecF *pos)
{
    OMOBJ *obj = capEffTrailOMObj;

    if (capEffTrailOMObj != NULL) {
        obj->trans.x = pos->x;
        obj->trans.y = pos->y;
        obj->trans.z = pos->z;
    }
}

// Removal and destination-highlight setup allocate a textured quad pool and its indexed draw list.
// Particle slots begin with zero scale so callers can activate them as their animations start.
static HU3D_MODELID CapEffCreate(ANIMDATA *anim, s16 num)
{
    CAP_EFFECT *effP;
    CAP_EFF_DATA *effDataP;
    s16 i;
    HuVec2f *st;
    HU3D_MODEL *modelP;
    HuVecF *vtx;
    HU3D_MODELID modelId;
    void *dlBuf;
    void *dlBegin;
    u32 workHeap;
    u32 particleHeap;
    u32 vertexHeap;
    u32 stHeap;
    u32 dlBufHeap;
    int dlSizeData;
    u32 dlDataHeap;
    void *workData;
    void *workBase;
    void *particleData;
    void *particleBase;
    void *vertexData;
    void *vertexBase;
    void *stData;
    void *stBase;
    void *dlBufData;
    void *dlBufBase;
    void *dlData;
    void *dlBase;

    modelId = Hu3DHookFuncCreate(CapEffDraw);
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    modelP = &Hu3DData[modelId];
    workHeap = modelP->mallocNo;
    workData = HuMemDirectMallocNum(HEAP_MODEL,
        sizeof(CAP_EFFECT), workHeap);
    workBase = workData;
    modelP->hookData = effP = workBase;
    effP->anim = anim;
    effP->num = num;
    effP->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    effP->dispAttr = CAP_EFF_DISPATTR_NONE;
    effP->hook = NULL;
    effP->hookMdlP = NULL;
    effP->count = 0;
    effP->attr = CAP_EFF_ATTR_NONE;
    effP->unusedByte = 0;
    effP->prevCount = 0;
    effP->mode = effP->time = 0;
    particleHeap = modelP->mallocNo;
    particleData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(CAP_EFF_DATA), particleHeap);
    particleBase = particleData;
    effP->data = effDataP = particleBase;
    memset(effDataP, 0, num * sizeof(CAP_EFF_DATA));
    for (i = 0; i < num; i++, effDataP++) {
        effDataP->scale = 0;
        effDataP->rot.x = effDataP->rot.y = effDataP->rot.z = 0;
        effDataP->animTime = 0;
        effDataP->animSpeed = 1;
        effDataP->pos.x = 0.0f;
        effDataP->pos.y = 0.0f;
        effDataP->pos.z = 0.0f;
        effDataP->color.r = effDataP->color.g = effDataP->color.b =
            effDataP->color.a = 255;
        effDataP->frameNo = 0;
    }
    vertexHeap = modelP->mallocNo;
    vertexData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(HuVecF) * 4, vertexHeap);
    vertexBase = vertexData;
    effP->vertex = vtx = vertexBase;
    for (i = 0; i < num * 4; i++, vtx++) {
        vtx->x = vtx->y = vtx->z = 0;
    }
    stHeap = modelP->mallocNo;
    stData = HuMemDirectMallocNum(HEAP_MODEL,
        num * sizeof(HuVec2f) * 4, stHeap);
    stBase = stData;
    effP->st = st = stBase;
    for (i = 0; i < num; i++) {
        st->x = 0;
        st->y = 0;
        st++;

        st->x = 1;
        st->y = 0;
        st++;

        st->x = 1;
        st->y = 1;
        st++;

        st->x = 0;
        st->y = 1;
        st++;
    }
    dlBufHeap = modelP->mallocNo;
    dlBufData = HuMemDirectMallocNum(HEAP_MODEL, 65536, dlBufHeap);
    dlBufBase = dlBufData;
    dlBegin = dlBuf = dlBufBase;
    DCFlushRange(dlBuf, 65536);
    GXBeginDisplayList(dlBegin, 65536);
    GXBegin(GX_QUADS, GX_VTXFMT0, num * 4);
    for (i = 0; i < num; i++) {
        GXPosition1x16(i * 4);
        GXColor1x16(i);
        GXTexCoord1x16(i * 4);

        GXPosition1x16((i * 4) + 1);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 1);

        GXPosition1x16((i * 4) + 2);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 2);

        GXPosition1x16((i * 4) + 3);
        GXColor1x16(i);
        GXTexCoord1x16((i * 4) + 3);
    }
    GXEnd();
    effP->dlSize = GXEndDisplayList();
    dlDataHeap = modelP->mallocNo;
    dlSizeData = effP->dlSize;
    dlData = HuMemDirectMallocNum(HEAP_MODEL, dlSizeData, dlDataHeap);
    dlBase = dlData;
    effP->dl = dlBase;
    memcpy(effP->dl, dlBuf, effP->dlSize);
    DCFlushRange(effP->dl, effP->dlSize);
    HuMemDirectFree(dlBuf);
    return modelId;
}

// The removal and highlight models' render hook builds textured quads from their particle slots.
// Display flags select facing, rotation, depth tests, and full-texture or sixteen-frame atlas UVs.
static void CapEffDraw(HU3D_MODEL *modelP, Mtx *mtx)
{
    CAP_EFFECT *effP;
    CAP_EFF_DATA *effDataP;
    HuVecF *vtx;
    HuVec2f *st;
    HuVecF *scaleVtxP;
    s16 i;
    s16 j;
    HuVecF *initVtxP;
    s16 bmpFmt;
    s16 row;
    s16 col;
    CAP_EFF_HOOK hook;

    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRotZ;
    HuVecF scaleVtx[4];
    HuVecF finalVtx[4];
    HuVecF initVtx[4];
    ROMtx basePosMtx;
    static HuVecF posTbl[4] = {
        { -0.5f,  0.5f, 0.0f },
        {  0.5f,  0.5f, 0.0f },
        {  0.5f, -0.5f, 0.0f },
        { -0.5f, -0.5f, 0.0f },
    };
    static HuVec2f uvTbl[4] = {
        { 0.0f, 0.0f },
        { 0.25f, 0.0f },
        { 0.25f, 0.25f },
        { 0.0f, 0.25f },
    };

    effP = modelP->hookData;
    if (effP->prevCounter != GlobalCounter || shadowModelDrawF) {
        if (effP->hookMdlP && effP->hookMdlP != modelP) {
            CapEffDraw(effP->hookMdlP, mtx);
        }
        GXLoadPosMtxImm(*mtx, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
            GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
            GX_COLOR0A0);
        if (shadowModelDrawF) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO,
                GX_CC_ZERO);
            GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        } else {
            bmpFmt = effP->anim->bmp->dataFmt & ANIM_BMP_FMTMASK;
            if (bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE,
                    GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC,
                    GX_CC_RASC, GX_CC_ZERO);
            }
            if (effP->dispAttr & CAP_EFF_DISPATTR_ZBUF_OFF) {
                GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
            } else if (modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA,
            GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
            GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX,
            GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
        HuSprTexLoad(effP->anim, 0, GX_TEXMAP0, GX_REPEAT, GX_REPEAT,
            GX_LINEAR);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(GX_FALSE);
        switch (effP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                    GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE,
                    GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR,
                    GX_LO_NOOP);
                break;
        }
        if (HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
            PSMTXIdentity(mtxInv);
        }
        PSMTXReorder(mtxInv, basePosMtx);
        if (effP->hook) {
            hook = effP->hook;
            hook(modelP, effP, mtx);
        }
        effDataP = effP->data;
        vtx = effP->vertex;
        st = effP->st;
        if (effP->dispAttr & CAP_EFF_DISPATTR_CAMERA_ROT) {
            MTXIdentity(mtxInv);
            MTXIdentity(*(Mtx *)(&basePosMtx));
            initVtx[0] = posTbl[0];
            initVtx[1] = posTbl[1];
            initVtx[2] = posTbl[2];
            initVtx[3] = posTbl[3];
        } else {
            PSMTXROMultVecArray(basePosMtx, &posTbl[0], initVtx, 4);
        }
        for (i = 0; i < effP->num; i++, effDataP++) {
            if (!effDataP->scale) {
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
                vtx->x = vtx->y = vtx->z = 0;
                vtx++;
            } else if (effP->dispAttr & CAP_EFF_DISPATTR_ROT3D) {
                VECScale(&posTbl[0], &scaleVtx[0], effDataP->scale);
                VECScale(&posTbl[1], &scaleVtx[1], effDataP->scale);
                VECScale(&posTbl[2], &scaleVtx[2], effDataP->scale);
                VECScale(&posTbl[3], &scaleVtx[3], effDataP->scale);
                mtxRot(mtxPos, effDataP->rot.x, effDataP->rot.y,
                    effDataP->rot.z);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            } else if (!effDataP->rot.z) {
                scaleVtxP = scaleVtx;
                initVtxP = initVtx;
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
                VECScale(initVtxP++, scaleVtxP, effDataP->scale);
                VECAdd(scaleVtxP++, &effDataP->pos, vtx++);
            } else {
                VECScale(&posTbl[0], &scaleVtx[0], effDataP->scale);
                VECScale(&posTbl[1], &scaleVtx[1], effDataP->scale);
                VECScale(&posTbl[2], &scaleVtx[2], effDataP->scale);
                VECScale(&posTbl[3], &scaleVtx[3], effDataP->scale);
                // This quad path takes radians, unlike the degree angles used for 3D highlights.
                MTXRotRad(mtxRotZ, 'Z', effDataP->rot.z);
                PSMTXConcat(mtxInv, mtxRotZ, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[1], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[2], &effDataP->pos, vtx++);
                VECAdd(&finalVtx[3], &effDataP->pos, vtx++);
            }
        }
        effDataP = effP->data;
        st = effP->st;
        if (!(effP->dispAttr & CAP_EFF_DISPATTR_NOANIM)) {
            for (i = 0; i < effP->num; i++, effDataP++) {
                row = effDataP->frameNo & 3;
                col = (effDataP->frameNo >> 2) & 3;
                for (j = 0; j < 4; j++, st++) {
                    st->x = (0.25f * row) + uvTbl[j].x;
                    st->y = (0.25f * col) + uvTbl[j].y;
                }
            }
        } else {
            // Nonanimated highlights use the entire texture instead of a quarter-width atlas cell.
            for (i = 0; i < effP->num; i++, effDataP++) {
                for (j = 0; j < 4; j++, st++) {
                    st->x = 4 * uvTbl[j].x;
                    st->y = 4 * uvTbl[j].y;
                }
            }
        }
        DCFlushRangeNoSync(effP->vertex, effP->num * sizeof(HuVecF) * 4);
        DCFlushRangeNoSync(effP->st, effP->num * sizeof(HuVec2f) * 4);
        DCFlushRangeNoSync(effP->data, effP->num * sizeof(CAP_EFF_DATA));
        PPCSync();
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, effP->vertex, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &effP->data->color, sizeof(CAP_EFF_DATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, effP->st, sizeof(HuVec2f));
        GXCallDisplayList(effP->dl, effP->dlSize);
        if (shadowModelDrawF == FALSE) {
            if (!(effP->attr & CAP_EFF_ATTR_COUNTER_UPDATE)) {
                effP->count++;
            }
            if (effP->prevCount != 0 && effP->prevCount <= effP->count) {
                if (effP->attr & CAP_EFF_ATTR_COUNTER_RESET) {
                    effP->count = 0;
                }
                effP->count = effP->prevCount;
            }
            effP->prevCounter = GlobalCounter;
        }
    }
}

// Human selection uses the signed heading difference to choose a route near the stick angle.
// Each input is wrapped by at most one turn before the difference is wrapped to 180 degrees.
static float CapAngleSumWrap(float targetAngle, float stickAngle)
{
    float result;

    if (targetAngle >= 360.0f) {
        targetAngle -= 360.0f;
    } else if (targetAngle < 0.0f) {
        targetAngle += 360.0f;
    }
    if (stickAngle >= 360.0f) {
        stickAngle -= 360.0f;
    } else if (stickAngle < 0.0f) {
        stickAngle += 360.0f;
    }
    result = targetAngle - stickAngle;
    if (result <= -180.0f) {
        result += 360.0f;
    } else if (result >= 180.0f) {
        result -= 360.0f;
    }
    return result;
}

// Converts a route heading through the current camera rotation for human stick selection.
static float CapCameraXZAngleGet(float angle)
{
    MBCAMERA *cameraP = mbCameraGet();
    Mtx cameraRotation;
    HuVecF heading;

    mtxRot(cameraRotation, cameraP->rot.x, cameraP->rot.y, cameraP->rot.z);
    heading.x = HuSin(angle);
    heading.y = 0.0f;
    heading.z = HuCos(angle);
    PSMTXMultVec(cameraRotation, &heading, &heading);
    return HuAtan(heading.x, heading.z);
}

// Computer capsule decisions attach a scripted left/right choice hook to the current board window.
static void CapComChoiceSet(int choice)
{
    capsuleComChoice = choice;
    mbWinTopComKeyHookSet(CapComKeyHook);
}

// The board window invokes this hook to queue the computer player's choice steps and A
// confirmation.
// Positive choices move right, negative choices move left, with the configured delay before each
// key.
static void CapComKeyHook(void)
{
    s32 key[4];
    s32 playerNo;
    s32 i;
    s32 padNo;
    s16 time;
    s32 keyValue;

    key[0] = key[1] = key[2] = key[3] = 0;
    playerNo = GwSystem.turnPlayerNo;
    padNo = GwPlayer[playerNo].padNo;
    time = GWComKeyDelayGet();
    if (capsuleComChoice >= 0) {
        keyValue = PAD_BUTTON_RIGHT;
    } else {
        keyValue = PAD_BUTTON_LEFT;
    }
    key[padNo] = keyValue;
    for (i = 0; i < abs(capsuleComChoice); i++) {
        key[padNo] = keyValue;
        HuWinComKeyWait(key[0], key[1], key[2], key[3], time);
    }
    key[padNo] = PAD_BUTTON_A;
    HuWinComKeyWait(key[0], key[1], key[2], key[3], time);
}

// Throw-path setup solves smooth cubic coefficients for one axis, with zero end curvature.
static inline void CalcThrowCameraParam(float *pos, float *paramOut, float *paramIn, int num)
{
    int i;
    float delta[8];
    float speed[8];

    paramOut[0] = 0;
    paramOut[num - 1] = 0;
    for (i = 0; i < num - 1; i++) {
        delta[i] = paramIn[i + 1] - paramIn[i];
        speed[i + 1] = (pos[i + 1] - pos[i]) / delta[i];
    }
    paramOut[1] = (speed[2] - speed[1]) - (delta[0] * paramOut[0]);
    speed[1] = 2 * (paramIn[2] - paramIn[0]);
    for (i = 1; i < num - 2; i++) {
        double t = delta[i] / speed[i];

        paramOut[i + 1] = (speed[i + 2] - speed[i + 1]) - (paramOut[i] * t);
        speed[i + 1] = (2 * (paramIn[i + 2] - paramIn[i])) - (delta[i] * t);
    }
    paramOut[num - 2] -= delta[num - 2] * paramOut[num - 1];
    for (i = num - 2; i > 0; i--) {
        paramOut[i] = (paramOut[i] - (delta[i] * paramOut[i + 1])) / speed[i];
    }
}

// A candidate throw path prepares the shared interpolation coefficients for all three axes.
// Point spacing uses only X and Y distances, even when Z changes along the throw.
static void CapThrowCameraSet(float *x, float *y, float *z, int num)
{
    int i;
    float dx;
    float dy;

    capsuleTime[0] = 0;
    for (i = 1; i < num; i++) {
        dx = x[i] - x[i - 1];
        dy = y[i] - y[i - 1];
        capsuleTime[i] = capsuleTime[i - 1] + sqrt((dx * dx) + (dy * dy));
    }
    for (i = 1; i < num; i++) {
        capsuleTime[i] /= capsuleTime[num - 1];
    }
    CalcThrowCameraParam(x, capsuleBezierX, capsuleTime, num);
    CalcThrowCameraParam(y, capsuleBezierY, capsuleTime, num);
    CalcThrowCameraParam(z, capsuleBezierZ, capsuleTime, num);
}

// Path search and flight playback evaluate the prepared throw curve at fraction t.
static void CapThrowCameraCalc(float t, float *x, float *y, float *z,
    HuVecF *out, int num)
{
    int lowX;
    int midX;
    int highX;
    int lowY;
    int midY;
    int highY;
    int lowZ;
    int midZ;
    int highZ;
    float spanX;
    float relX;
    float spanY;
    float relY;
    float spanZ;
    float relZ;
    float outX;
    float outY;
    float outZ;

    lowX = 0;
    highX = num - 1;
    while (lowX < highX) {
        midX = (lowX + highX) / 2;
        if (capsuleTime[midX] < t) {
            lowX = midX + 1;
        } else {
            highX = midX;
        }
    }
    if (lowX > 0) {
        lowX--;
    }
    spanX = capsuleTime[lowX + 1] - capsuleTime[lowX];
    relX = t - capsuleTime[lowX];
    outX = x[lowX] + (relX *
        ((relX * ((3.0f * capsuleBezierX[lowX]) +
            ((relX * (capsuleBezierX[lowX + 1] - capsuleBezierX[lowX])) /
                spanX))) +
            (((x[lowX + 1] - x[lowX]) / spanX) -
                (spanX * ((2.0f * capsuleBezierX[lowX]) +
                    capsuleBezierX[lowX + 1])))));
    out->x = outX;

    lowY = 0;
    highY = num - 1;
    while (lowY < highY) {
        midY = (lowY + highY) / 2;
        if (capsuleTime[midY] < t) {
            lowY = midY + 1;
        } else {
            highY = midY;
        }
    }
    if (lowY > 0) {
        lowY--;
    }
    spanY = capsuleTime[lowY + 1] - capsuleTime[lowY];
    relY = t - capsuleTime[lowY];
    outY = y[lowY] + (relY *
        ((relY * ((3.0f * capsuleBezierY[lowY]) +
            ((relY * (capsuleBezierY[lowY + 1] - capsuleBezierY[lowY])) /
                spanY))) +
            (((y[lowY + 1] - y[lowY]) / spanY) -
                (spanY * ((2.0f * capsuleBezierY[lowY]) +
                    capsuleBezierY[lowY + 1])))));
    out->y = outY;

    lowZ = 0;
    highZ = num - 1;
    while (lowZ < highZ) {
        midZ = (lowZ + highZ) / 2;
        if (capsuleTime[midZ] < t) {
            lowZ = midZ + 1;
        } else {
            highZ = midZ;
        }
    }
    if (lowZ > 0) {
        lowZ--;
    }
    spanZ = capsuleTime[lowZ + 1] - capsuleTime[lowZ];
    relZ = t - capsuleTime[lowZ];
    outZ = z[lowZ] + (relZ *
        ((relZ * ((3.0f * capsuleBezierZ[lowZ]) +
            ((relZ * (capsuleBezierZ[lowZ + 1] - capsuleBezierZ[lowZ])) /
                spanZ))) +
            (((z[lowZ + 1] - z[lowZ]) / spanZ) -
                (spanZ * ((2.0f * capsuleBezierZ[lowZ]) +
                    capsuleBezierZ[lowZ + 1])))));
    out->z = outZ;
}

// Board collision setup resolves its hidden board model to the engine model used for tests.
static void CapColMdlIdGet(void)
{
    int boardNo = MBBoardNoGet();

    if (capsuleColObjId != MB_MODEL_NONE) {
        capsuleColMdlId = mbObjModelIDGet(capsuleColObjId);
    }
}

// Throw-path search tests its segment against triangles in the hidden board collision model.
// Plane crossings update out even when the final point lies outside the triangle.
static BOOL CapColCheck(HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    HSF_FACE *faceP;
    HSF_DATA *hsfP;
    HSF_BUFFER *faceBufP;
    HSF_BUFFER *vtxBufP;
    BOOL quadF;
    int i;
    int j;
    HU3D_MODEL *modelP;
    float dot;
    float dotEdge;
    float dotA;
    float dotB;
    float invDotA;
    float scale;
    HuVecF faceVtx[4];
    HuVecF faceNorm;
    HuVecF faceCA;
    HuVecF faceBA;
    HuVecF faceBAQuad;
    HuVecF faceCAQuad;
    HuVecF faceNormQuad;
    HuVecF dir;
    HuVecF outPos;
    HuVecF dotVec;
    float mag;
    int temp;

    quadF = FALSE;
    temp = 0;
    if (capsuleColMdlId == HU3D_MODELID_NONE) {
        return FALSE;
    }
    modelP = &Hu3DData[capsuleColMdlId];
    hsfP = modelP->hsf;
    faceBufP = hsfP->face;
    vtxBufP = hsfP->vertex;
    PSVECSubtract(posA, posB, &dir);
    if (PSVECMag(&dir) <= 0.0f) {
        return FALSE;
    }
    PSVECNormalize(&dir, &dir);
    mag = PSVECMag(&dir);
    for (i = 0; i < hsfP->faceNum; i++, faceBufP++) {
        faceP = faceBufP->data;
        for (j = 0; j < faceBufP->count; j++, faceP++) {
            switch (faceP->type & HSF_FACE_MASK) {
            case 0:
                quadF = TRUE;
                break;

            case 1:
                quadF = TRUE;
                break;

            case HSF_FACE_TRI:
                quadF = FALSE;
                faceVtx[0] =
                    *((HuVecF *)vtxBufP->data + faceP->index[0].vertex);
                faceVtx[1] =
                    *((HuVecF *)vtxBufP->data + faceP->index[1].vertex);
                faceVtx[2] =
                    *((HuVecF *)vtxBufP->data + faceP->index[2].vertex);
                break;

            case HSF_FACE_QUAD:
                quadF = TRUE;
                break;

            default:
                break;
            }
            if (!quadF) {
                PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBAQuad);
                PSVECSubtract(&faceVtx[2], &faceVtx[0], &faceCAQuad);
                PSVECCrossProduct(&faceBAQuad, &faceCAQuad, &faceNormQuad);
                if (PSVECMag(&faceNormQuad) > 0.0f) {
                    PSVECNormalize(&faceNormQuad, &faceNormQuad);
                }
                dot = -PSVECDotProduct(&faceNormQuad, &faceVtx[0]);
                dotA = (faceNormQuad.x * posA->x)
                    + (faceNormQuad.y * posA->y)
                    + (faceNormQuad.z * posA->z) + dot;
                dotB = (faceNormQuad.x * posB->x)
                    + (faceNormQuad.y * posB->y)
                    + (faceNormQuad.z * posB->z) + dot;
                if (dotA * dotB > 0) {
                    continue;
                }
                invDotA = -((faceNormQuad.x * posA->x)
                    + (faceNormQuad.y * posA->y)
                    + (faceNormQuad.z * posA->z) + dot);
                dotEdge = (faceNormQuad.x * dir.x)
                    + (faceNormQuad.y * dir.y)
                    + (faceNormQuad.z * dir.z);
                if (dotEdge != 0.0f) {
                    scale = invDotA / dotEdge;
                    PSVECScale(&dir, &outPos, scale);
                    PSVECAdd(&outPos, posA, &outPos);
                    *out = outPos;
                    PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[0], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.x = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    PSVECSubtract(&faceVtx[2], &faceVtx[1], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[1], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.y = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    PSVECSubtract(&faceVtx[0], &faceVtx[2], &faceBA);
                    PSVECSubtract(&outPos, &faceVtx[2], &faceCA);
                    PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
                    dotVec.z = PSVECDotProduct(&faceNormQuad, &faceNorm);
                    if (dotVec.x > 0 && dotVec.y > 0 && dotVec.z > 0) {
                        return TRUE;
                    }
                    if (dotVec.x < 0 && dotVec.y < 0 && dotVec.z < 0) {
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}

static GXColor capsuleCrackEffColor = { 255, 255, 128, 64 };

static GXColor capsuleCrackEffAmbColor = { 255, 255, 255, 255 };

static GXColor capsuleCrackEffMatColor = { 255, 255, 255, 255 };

static void CapColKill(void)
{
    capsuleColMdlId = MB_MODEL_NONE;
}

static HuVecF colScaleTbl[18] = {
    { -0.5f, 0.0f, 0.5f },
    { -0.5f, 1.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { -0.5f, 0.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.5f, 1.0f, 0.5f },
    { 0.0f, 1.0f, -0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.0f, 1.0f, -0.5f },
    { 0.0f, 0.0f, -0.5f },
    { 0.0f, 0.0f, -0.5f },
    { 0.0f, 1.0f, -0.5f },
    { -0.5f, 1.0f, 0.5f },
    { 0.0f, 0.0f, -0.5f },
    { -0.5f, 1.0f, 0.5f },
    { -0.5f, 0.0f, 0.5f },
};

static HuVec2f charSizeTbl[14] = {
    { 100.0f, 175.0f },
    { 100.0f, 200.0f },
    { 110.0f, 220.0f },
    { 100.0f, 175.0f },
    { 175.0f, 175.0f },
    { 110.0f, 220.0f },
    { 200.0f, 260.0f },
    { 100.0f, 150.0f },
    { 175.0f, 175.0f },
    { 150.0f, 125.0f },
    { 100.0f, 150.0f },
    { 150.0f, 125.0f },
    { 150.0f, 125.0f },
    { 150.0f, 125.0f },
};

// Throw-path search tests a segment against six triangular faces around each other player.
// Hidden players are skipped; solo mode considers only player zero.
static BOOL CapColExec(int playerNo, HuVecF *posA, HuVecF *posB, HuVecF *out)
{
    HuVecF faceVtx[4];
    HuVecF edgeNorm;
    HuVecF edgeCA;
    HuVecF edgeBA;
    HuVecF faceBA;
    HuVecF faceCA;
    HuVecF faceNorm;
    HuVecF dir;
    HuVecF outPos;
    HuVecF playerPos;
    float dot;
    float dotEdge;
    float dotA;
    float dotB;
    float invDotA;
    float scale;
    float mag;
    float dotVec[3];
    BOOL partyF;
    int i;
    int j;
    int temp;
    int temp2;

    temp = 0;
    temp2 = 0;
    PSVECSubtract(posA, posB, &dir);
    if (PSVECMag(&dir) <= 0.0f) {
        return FALSE;
    }
    PSVECNormalize(&dir, &dir);
    mag = PSVECMag(&dir);
    for (i = 0; i < 4; i++) {
        partyF = GwSystem.partyF;
        if ((!partyF && i != 0) || i == playerNo ||
            !mbPlayerDispGet(i)) {
            continue;
        }
        mbPlayerPosGet(i, &playerPos);
        for (j = 0; j < 6; j++) {
            faceVtx[0].x = colScaleTbl[j * 3].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[0].y = colScaleTbl[j * 3].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[0].z = colScaleTbl[j * 3].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[1].x = colScaleTbl[j * 3 + 1].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[1].y = colScaleTbl[j * 3 + 1].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[1].z = colScaleTbl[j * 3 + 1].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[2].x = colScaleTbl[j * 3 + 2].x *
                charSizeTbl[GwPlayer[i].charNo].x;
            faceVtx[2].y = colScaleTbl[j * 3 + 2].y *
                charSizeTbl[GwPlayer[i].charNo].y;
            faceVtx[2].z = colScaleTbl[j * 3 + 2].z *
                charSizeTbl[GwPlayer[i].charNo].x;
            PSVECAdd(&faceVtx[0], &playerPos, &faceVtx[0]);
            PSVECAdd(&faceVtx[1], &playerPos, &faceVtx[1]);
            PSVECAdd(&faceVtx[2], &playerPos, &faceVtx[2]);
            PSVECSubtract(&faceVtx[1], &faceVtx[0], &faceBA);
            PSVECSubtract(&faceVtx[2], &faceVtx[0], &faceCA);
            PSVECCrossProduct(&faceBA, &faceCA, &faceNorm);
            if (PSVECMag(&faceNorm) > 0.0f) {
                PSVECNormalize(&faceNorm, &faceNorm);
            }
            dot = -PSVECDotProduct(&faceNorm, &faceVtx[0]);
            dotA = (faceNorm.x * posA->x) +
                (faceNorm.y * posA->y) + (faceNorm.z * posA->z) + dot;
            dotB = (faceNorm.x * posB->x) +
                (faceNorm.y * posB->y) + (faceNorm.z * posB->z) + dot;
            if (dotA * dotB > 0.0f) {
                continue;
            }
            invDotA = -((faceNorm.x * posA->x) +
                (faceNorm.y * posA->y) + (faceNorm.z * posA->z) + dot);
            dotEdge = (faceNorm.x * dir.x) +
                (faceNorm.y * dir.y) + (faceNorm.z * dir.z);
            if (dotEdge == 0.0f) {
                continue;
            }
            scale = invDotA / dotEdge;
            PSVECScale(&dir, &outPos, scale);
            PSVECAdd(&outPos, posA, &outPos);
            *out = outPos;
            PSVECSubtract(&faceVtx[1], &faceVtx[0], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[0], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[0] = PSVECDotProduct(&faceNorm, &edgeNorm);
            PSVECSubtract(&faceVtx[2], &faceVtx[1], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[1], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[1] = PSVECDotProduct(&faceNorm, &edgeNorm);
            PSVECSubtract(&faceVtx[0], &faceVtx[2], &edgeBA);
            PSVECSubtract(&outPos, &faceVtx[2], &edgeCA);
            PSVECCrossProduct(&edgeBA, &edgeCA, &edgeNorm);
            dotVec[2] = PSVECDotProduct(&faceNorm, &edgeNorm);
            if (dotVec[0] > 0.0f && dotVec[1] > 0.0f && dotVec[2] > 0.0f) {
                return TRUE;
            }
            if (dotVec[0] < 0.0f && dotVec[1] < 0.0f && dotVec[2] < 0.0f) {
                return TRUE;
            }
        }
    }
    return FALSE;
}
