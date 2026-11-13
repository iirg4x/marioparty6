#include "REL/framework/random.h"
#include "REL/m672DLL/controllers.h"
#include "humath.h"
#include "REL/framework/sphere.h"
extern "C" {
#include <math.h>
#include "game/gamework.h"
#include "dolphin.h"
#include "REL/framework/vector_math.h"
#include "game/pad.h"
#include "game/board/player.h"
}
#include "REL/m672DLL/player.h"
#include "math.h"
// Performs no additional work when the player controller is destroyed.
inline PlayerController::~PlayerController() {}

// Clears the controller values and invokes the controller update callback.
void fn_1_758(PlayerController *state)
{
    state->values.x = 0.0f;
    state->values.y = 0.0f;
    state->values.magnitude = 0.0f;
    state->update(&state->values);
}

// Initializes pad-angle state, resolves the pad number, and selects a random mode bit.
PadController::PadController(Player *owner) : PlayerController(owner)
{
    angle = 0.0f;
    angleStep = 2.0f;
    s32 playerIndex = owner->playerNo;
    padNumber = lbl_80265728[playerIndex].padNo;
    values.mode = RandomUnsigned() & 1;
}
extern "C" {

const Vec lbl_1_rodata_18[2] = {{0.0f, 0.0f, 90.0f}, {0.0f, 0.0f, -90.0f}};

}

// Returns the game-player pointer held by the Player object.
static inline MGPLAYER *PadPlayer(Player *owner)
{
    return owner->player;
}

// Builds the pad direction, optionally submits pad values, and stores direction/buttons in state.
void PadController::update(PadState *state)
{
    EffectVector direction;
    Mtx angleMatrix;
    Mtx tiltMatrix;
    s32 padIndex;
    f32 angleWobble;
    double length;

    padIndex = padNumber;
    // Construction sets mode to 0 or 1; the alternate path accepts -1.
    if (state->mode != -1) {
        direction.set(lbl_802BFFFC[padIndex], lbl_802BFFF8[padIndex], 0.0);
        PSMTXRotRad(angleMatrix, 'z', MTXDegToRad(lbl_1_rodata_18[state->mode].z));
        angleWobble = (f32)(40.0 * sin((M_PI * (f64)angle) / 180.0));
        angle += angleStep;
        if (angle > 360.0f) {
            angle = 0.0f;
            angleStep = RandomFloat(2.0f, 4.0f);
        }
        PSMTXRotRad(tiltMatrix, 'z', MTXDegToRad(angleWobble));
        PSMTXConcat(angleMatrix, tiltMatrix, angleMatrix);
        PSMTXMultVec(angleMatrix, &direction.value, &direction.value);
        PSVECScale(&direction.value, &direction.value, 0.45f);
        fn_80080520(PadPlayer(player), (s32)direction.value.x,
                       (s32)direction.value.y, lbl_802C0008[padIndex], lbl_802C0010[padIndex]);
    } else {
        direction.set(lbl_8024C7D8[padIndex], lbl_8024C7E8[padIndex], 0.0);
    }
    // Normalize only when both horizontal components are nonzero.
    if (direction.value.x != 0.0f && direction.value.y != 0.0f) {
        PSVECNormalize(&direction.value, &direction.value);
    }
    length = vectorMagnitude(&direction.value);
    state->x = direction.value.x;
    state->y = direction.value.y;
    state->magnitude = length;
    state->buttonsDown = lbl_802C0008[padIndex];
    state->buttons = lbl_802C0010[padIndex];
}

// Initializes difficulty, timing, callback, destination, and the inherited controller state.
ComController::ComController(Player *owner, int difficultyValue)
    : PlayerController(owner)
{
    difficulty = difficultyValue;
    elapsed = 0.0f;
    angle = 0.0f;
    angleStep = 2.0f;
    setUpdate(&ComController::fn_1_EA0);
    destination.x = 0.0f;
    destination.y = 0.0f;
    destination.z = -2880.0f;
}

// Performs no additional work when the computer controller is destroyed.
ComController::~ComController() {}
// Runs the selected callback, submits the input to the current player, and updates pad state.
void ComController::update(PadState *state)
{
 Player *source;
 MGPLAYER *currentPlayer;
 MGPLAYER *actionPlayer;
 if (callback) { (this->*callback)(state); }
 source=player;
 currentPlayer=source->player;
 actionPlayer=currentPlayer;
 input.fn_1_12DA0(actionPlayer);
}

// Empty initial computer-controller callback.
void ComController::fn_1_EA0(PadState *) {}
extern "C" {
const float lbl_1_rodata_6C[4] = {0.5f,1.0f,1.5f,2.0f};
}
// Steers toward the destination with periodic destination changes and bounded angular wobble.
void ComController::fn_1_EA4(PadState *)
{
    Player *target = player;
    Vec position = target->positionValue();
    Vec direction;
    Mtx rotation;
    f32 rotationScale;
    f32 angularOffset;

    if (elapsed > lbl_1_rodata_6C[difficulty]) {
        fn_1_10B8();
        elapsed = 0.0f;
    }

    rotationScale = 9.333333f;
    PSVECSubtract(&destination, &position, &direction);
    direction.y = 0.0f;
    PSVECNormalize(&direction, &direction);

    angularOffset =
        (f32)(40.0 *
              sin((M_PI * angle) /
                  180.0));

    angle += angleStep;
    if (angle > 360.0f) {
        angle = 0.0f;
        angleStep = RandomFloat(2.0f,
                                               4.0f);
    }

    PSMTXRotRad(rotation, 'y', MTXDegToRad(angularOffset));
    PSMTXMultVec(rotation, &direction, &direction);
    PSVECScale(&direction, &direction, 0.45f * rotationScale);
    input.fn_1_12B70(&direction, 0.1f);
    elapsed += (1.0 / 60.0);
}

extern "C" {
const float lbl_1_rodata_90[4]={400.0f,300.0f,200.0f,100.0f};
const float lbl_1_rodata_A0[4]={400.0f,500.0f,600.0f,700.0f};
const float lbl_1_rodata_B0[4]={0.8f,0.6f,0.4f,0.2f};
}
// Chooses a difficulty-scaled random destination from the player's current position.
void ComController::fn_1_10B8()
{
    Player *state;
    f32 scale;
    f32 magnitude;
    f32 angleMagnitude;
    f32 angularRange;
    f32 scaleResult;
    double scaledValue;
    double rotation;
    s32 difficultyIndex;

    difficultyIndex = difficulty;
    state = player;
    Vec parameters = state->positionValue();

    if ((200.0f + parameters.z) < 0.0f) {
        magnitude = -(200.0f + parameters.z);
    } else {
        magnitude = 200.0f + parameters.z;
    }
    scale = 1.0f - (magnitude / 2700.0f);
    if (scale < lbl_1_rodata_B0[difficultyIndex]) {
        scale = lbl_1_rodata_B0[difficultyIndex];
    }
    if (scale > 1.0f) {
        scale = 1.0f;
    }

    if (parameters.z < 0.0f) {
        angleMagnitude = -parameters.z;
    } else {
        angleMagnitude = parameters.z;
    }
    angularRange = angleMagnitude - lbl_1_rodata_90[difficultyIndex];
    scaleResult = lbl_1_rodata_A0[difficultyIndex] +
                  (parameters.z < 0.0f
                       ? -parameters.z
                       : parameters.z);
    rotation = -RandomFloat(angularRange, scaleResult);
    scaledValue = scale * RandomFloat(-900.0f, 900.0f);
    destination.x = (f32)scaledValue;
    destination.y = 0.0f;
    destination.z = (f32)rotation;
}

// Selects the active computer-controller callback.
void ComController::reset() { setUpdate(&ComController::fn_1_EA4); }
// Performs no additional work when the pad controller is destroyed.
inline PadController::~PadController() {}

// Empty base reset hook.
inline void PlayerController::reset() {}
