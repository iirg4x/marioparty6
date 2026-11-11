#include "REL/m672DLL/random.h"
#include "REL/m672DLL/controllers_bf64.h"
#include "humath.h"
extern "C" {
#include <math.h>
#include "game/gamework.h"
#include "dolphin.h"
#include "game/pad.h"
#include "game/board/player.h"
}
#include "REL/m672DLL/player.h"
#include "math.h"
inline PlayerController::~PlayerController() {}

void fn_1_758(PlayerController *state)
{
    state->values.x = 0.0f;
    state->values.y = 0.0f;
    state->values.magnitude = 0.0f;
    state->update(&state->values);
}

PadController::PadController(Player *owner) : PlayerController(owner)
{
    angle = 0.0f;
    angleStep = 2.0f;
    s32 playerIndex = owner->playerNo;
    padNumber = lbl_80265728[playerIndex].padNo;
    values.mode = RandomUnsigned() & 1;
}
extern "C" {

typedef struct PadOwner8F4 {
    u8 unk_0[0x20]; /* No code names this field. */
    void *player;
    u8 unk_24[0x24]; /* No code names this field. */
    f32 angle;
    f32 angleStep;
    s32 padIndex;
} PadOwner8F4;

typedef struct PadAngle8F4 {
    u8 unknown_00[8]; /* No code names this field. */
    f32 angle;
} PadAngle8F4;

const PadAngle8F4 lbl_1_rodata_18[2] = {{{0},90.0f},{{0},-90.0f}};

}

static inline void SetDirection8F4(Vec *direction, double x, double y)
{
    direction->x = x;
    direction->y = y;
    direction->z = 0.0f;
}

static inline MGPLAYER *PadPlayer8F4(Player *owner)
{
    return owner->player;
}

static inline double Magnitude8F4(Vec *direction)
{
    return PSVECMag(direction);
}

void PadController::update(PadState8F4 *state)
{
    Vec direction;
    Mtx angleMatrix;
    Mtx tiltMatrix;
    s32 padIndex;
    f32 angleWobble;
    double length;

    padIndex = padNumber;
    if (state->mode != -1) {
        SetDirection8F4(&direction, lbl_802BFFFC[padIndex], lbl_802BFFF8[padIndex]);
        PSMTXRotRad(angleMatrix, 'z', MTXDegToRad(lbl_1_rodata_18[state->mode].angle));
        angleWobble = (f32)(40.0 * sin((M_PI * (f64)angle) / 180.0));
        angle += angleStep;
        if (angle > 360.0f) {
            angle = 0.0f;
            angleStep = RandomFloat(2.0f, 4.0f);
        }
        PSMTXRotRad(tiltMatrix, 'z', MTXDegToRad(angleWobble));
        PSMTXConcat(angleMatrix, tiltMatrix, angleMatrix);
        PSMTXMultVec(angleMatrix, &direction, &direction);
        PSVECScale(&direction, &direction, 0.45f);
        fn_80080520(PadPlayer8F4(player), (s32)direction.x,
                       (s32)direction.y, lbl_802C0008[padIndex], lbl_802C0010[padIndex]);
    } else {
        SetDirection8F4(&direction, lbl_8024C7D8[padIndex], lbl_8024C7E8[padIndex]);
    }
    if (direction.x != 0.0f && direction.y != 0.0f) {
        PSVECNormalize(&direction, &direction);
    }
    length = Magnitude8F4(&direction);
    state->x = direction.x;
    state->y = direction.y;
    state->magnitude = length;
    state->buttonsDown = lbl_802C0008[padIndex];
    state->buttons = lbl_802C0010[padIndex];
}

ComController::ComController(Player *owner, int difficultyValue)
    : PlayerController(owner)
{
    difficulty = difficultyValue;
    elapsed = 0.0f;
    angle = 0.0f;
    angleStep = 2.0f;
    void (ComController::*initial)(PadState8F4 *) = &ComController::fn_1_EA0;
    callback = initial;
    destination.x = 0.0f;
    destination.y = 0.0f;
    destination.z = -2880.0f;
}

ComController::~ComController() {}
void ComController::update(PadState8F4 *state)
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

void ComController::fn_1_EA0(PadState8F4 *) {}
extern "C" {
const float lbl_1_rodata_6C[4] = {0.5f,1.0f,1.5f,2.0f};
}
void ComController::fn_1_EA4(PadState8F4 *)
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
    elapsed += 0.016666666666666666;
}

extern "C" {
const float lbl_1_rodata_90[4]={400.0f,300.0f,200.0f,100.0f};
const float lbl_1_rodata_A0[4]={400.0f,500.0f,600.0f,700.0f};
const float lbl_1_rodata_B0[4]={0.8f,0.6f,0.4f,0.2f};
}
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

void ComController::reset() { void (ComController::*initial)(PadState8F4 *) = &ComController::fn_1_EA4; callback=initial; }
inline PadController::~PadController() {}

inline void PlayerController::reset() {}
