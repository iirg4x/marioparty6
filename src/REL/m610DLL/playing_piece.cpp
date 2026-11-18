#include "math.h"

extern "C"
{
#include "dolphin/os.h"
}

#include "dolphin/mtx.h"

extern "C"
{
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/wipe.h"
#include "game/mg/timer.h"
}

#include "REL/framework/world_units.h"
#include "REL/framework/frame_time.h"
#include "REL/m610DLL/effect_functions.h"
#include "REL/m610DLL/particle_emitter.h"
#include <functional>
#include "REL/m610DLL/module_types.h"
#include "dolphin/types.h"
#include "datadir_enum.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/model.h"
#include "REL/framework/octtree_man.h"
#include "REL/m610DLL/rigid_box.h"
extern "C"
{
#include "game/hu3d.h"
}
#include "game/gamework.h"

#include "REL/m610DLL/playing_piece.h"
#include "REL/framework/event_callback_registration.h"

/* Create the playing piece, its box, six models and input callbacks. */
PlayerPiece::PlayerPiece(OwnedModel *animation, OwnedModel *modelSource,
                         TriangleCollision *collision, PlayerInfo *player,
                         ParticleEmitterState *child)
    : GameObject(0), animation(animation), collision(collision), player(player),
      box(new RigidBox(collision, player)), callbacks(2), child(child)
{
    models[0].reset(modelSource);
    models[1].reset(new OwnedModel(*modelSource));
    models[2].reset(new OwnedModel(*modelSource));
    models[3].reset(new OwnedModel(*modelSource));
    models[4].reset(new OwnedModel(*modelSource));
    models[5].reset(new OwnedModel(*modelSource));
    for (int index = 0; index < 6; ++index)
    {
        models[index]->setShadow();
        models[index]->setLayer(5);
    }
    registerEventCallback(&callbacks, std::bind1st(std::mem_fun(&PlayerPiece::fn_1_276C), this), 2);
    registerEventCallback(&callbacks, std::bind1st(std::mem_fun(&PlayerPiece::fn_1_2778), this), 3);
    inputEnabled = 0;
    crossedDepthLimit = 0;
    this->animation->setLayer(5);
    auxiliaryModel.reset(new OwnedModel(this->player->characterNo() + DATANUM(0x450000, 31)));
    auxiliaryModel->setLayer(5);
    horizontalAngle = 0.0f;
    verticalAngle = 0.0f;
    fn_1_2784(0.0f, 0.0f);
    remainingSteps = secondsToFrames<long>(3);
}

// Read here, after the constructor and in this order, these two definitions set where the module's
// singleton pointers go and the order of its weak functions.
#include "REL/framework/game_object_man.h"
#include "REL/m610DLL/geometry_singleton.h"

void fn_1_1E90(void);

void fn_1_1E94(void);

/* Install the empty game-object update hooks. */
void fn_1_1E54(void)
{
    setPreUpdateHook(0, fn_1_1E90);
    setPostUpdateHook(0, fn_1_1E94);
}

/* Leave the pre-update hook empty. */
void fn_1_1E90(void)
{
}

/* Leave the post-update hook empty. */
void fn_1_1E94(void)
{
}

/* Cache whether the source position has crossed the negative depth limit. */
bool PlayerPiece::fn_1_1E98()
{
    Vector3 position;
    if (crossedDepthLimit)
        return true;
    box.get()->fn_1_A5EC(&position);
    crossedDepthLimit = position.z < -3800.0f;
    return crossedDepthLimit;
}

/* Copy the playing piece translation from its matrix. */
void PlayerPiece::fn_1_1F10(Vec *position) const
{
    position->x = this->matrix[0][3];
    position->y = this->matrix[1][3];
    position->z = this->matrix[2][3];
}

/* Copy the forward direction from the playing piece matrix. */
void PlayerPiece::fn_1_1F2C(Vec *direction) const
{
    direction->x = this->matrix[0][2];
    direction->y = this->matrix[1][2];
    direction->z = this->matrix[2][2];
}

/* Copy the negated lateral direction from the playing piece matrix. */
void PlayerPiece::fn_1_1F48(Vec *normal) const
{
    normal->x = -this->matrix[0][0];
    normal->y = -this->matrix[1][0];
    normal->z = -this->matrix[2][0];
}

/* Reset the box corners previous positions. */
void PlayerPiece::fn_1_1F70()
{
    box->fn_1_9EE4();
}

/* Place the box at the supplied position. */
void PlayerPiece::fn_1_1FAC(const Vec *data)
{
    box->fn_1_81D4(data);
}

/* Advance input, box constraints, model transforms and the attached effect. */
void PlayerPiece::update()
{
    RigidBox *rigidBox;

    if (remainingSteps > 0)
    {
        --remainingSteps;
        box.get()->fn_1_A048(0.2f);
    }

    fn_1_29E0();
    rigidBox = box.get();
    rigidBox->fn_1_89A8();
    rigidBox->fn_1_998C(6.0f * horizontalStick);
    rigidBox->fn_1_9C38(6.0f * verticalStick);
    rigidBox->fn_1_8B04();
    rigidBox->fn_1_89A4();
    fn_1_20D8();
    fn_1_21C4();
    fn_1_24A8();
}

/* Build the box transform and apply the opening depth offset. */
void PlayerPiece::fn_1_20D8()
{
    float frameFraction;
    float verticalTranslation;

    box.get()->fn_1_A284(matrix);
    frameFraction = remainingSteps / secondsToFrames<float>(3.0f);
    verticalTranslation = frameFraction * cmToWorld(800.0f);
    PSMTXTransApply(matrix, matrix, 0.0f, 0.0f, verticalTranslation);
    animation->copy_matrix(matrix);
}

/* Offsets of the six models around the playing piece. */
const Vec lbl_1_rodata_110[6] = { { 84.0f, 40.0f, 98.0f },  { 84.0f, 40.0f, 6.0f },
                                  { 84.0f, 40.0f, -82.0f }, { -84.0f, 40.0f, 98.0f },
                                  { -84.0f, 40.0f, 6.0f },  { -84.0f, 40.0f, -82.0f } };

/* Build and apply the six rotated effect matrices and their point transforms. */
void PlayerPiece::fn_1_21C4()
{
    Mtx firstRotation;
    Mtx secondRotation;
    Mtx matrix;
    int index = 0;
    float angle;
    float pointDistance;
    float finalTranslationY;
    float finalTranslationZ;
    float translationX;
    float translationY;
    float translationZ;

    while (index < 6)
    {
        PSMTXRotRad(firstRotation, 'Y', MTXDegToRad((float) (index >= 3 ? 180 : 0)));
        angle = index >= 3 ? -verticalAngle : horizontalAngle;
        PSMTXRotRad(secondRotation, 'X', MTXDegToRad(angle));
        PSMTXConcat(firstRotation, secondRotation, matrix);
        translationZ = lbl_1_rodata_110[index].z;
        translationY = lbl_1_rodata_110[index].y;
        translationX = lbl_1_rodata_110[index].x;
        PSMTXTransApply(matrix, matrix, translationX, translationY, translationZ);
        PSMTXConcat(this->matrix, matrix, matrix);
        models[index]->copy_matrix(matrix);

        vectorSet(&entries[index].center, 0.0, 0.0, 0.0);
        PSMTXMultVec(matrix, &entries[index].center, &entries[index].center);
        pointDistance = 150.0f;
        entries[index].setRadius(pointDistance);
        ++index;
    }

    PSMTXRotRad(matrix, 'Y', 0.0f);
    finalTranslationZ = 0.0f;
    finalTranslationY = 61.0f;
    PSMTXTransApply(matrix, matrix, 0.0f, finalTranslationY, finalTranslationZ);
    PSMTXConcat(this->matrix, matrix, matrix);
    if (auxiliaryModel.get() != 0)
    {
        auxiliaryModel->copy_matrix(matrix);
    }
}

/* Assign the supplied vector coordinates. */
static inline void setVectorCoordinates(Vec *out, f32 x, f32 y, f32 z)
{
    Vec coordinates;
    coordinates.x = x;
    coordinates.y = y;
    coordinates.z = z;
    out->x = coordinates.x;
    out->y = coordinates.y;
    out->z = coordinates.z;
}
/* Copy the negated coordinates of a vector. */
static inline void negateVectorCoordinates(Vec *out, const Vec *p)
{
    setVectorCoordinates(out, -p->x, -p->y, -p->z);
}
/* Move and direct the attached particles and adjust the sliding sound. */
void PlayerPiece::fn_1_24A8()
{
    Vec worldPosition;
    Vector3 direction;
    f32 scale, scaleX, scaleY, xMagnitude, yMagnitude;
    double worldX, worldY, worldZ;
    RigidBox *rigidBox;
    Vec *boxNormal;
    f32 distance;
    worldZ = -130.0;
    worldY = 170.0;
    worldX = 45.0;
    worldPosition.x = (f32) worldX;
    worldPosition.y = (f32) worldY;
    worldPosition.z = (f32) worldZ;
    PSMTXMultVec(this->matrix, &worldPosition, &worldPosition);
    child->fn_1_13248(&worldPosition);
    scaleX = this->horizontalStick;
    if (scaleX < 0.0f)
        xMagnitude = -scaleX;
    else
        xMagnitude = scaleX;
    scaleY = this->verticalStick;
    if (scaleY < 0.0f)
        yMagnitude = -scaleY;
    else
        yMagnitude = scaleY;
    scale = 0.1f + (xMagnitude + yMagnitude) * 0.9f / 2.0f;
    child->fn_1_13280(scale);
    if (this->remainingSteps <= 0)
    {
        rigidBox = box.get();
        if (scale <= 0.11f)
            rigidBox->fn_1_A0BC();
        else
            rigidBox->fn_1_A048(scale);
    }
    boxNormal = &box.get()->normalZ;
    negateVectorCoordinates(&direction, boxNormal);
    direction.y += 0.6f;
    distance = direction.length();
    PSVECScale(&direction, &direction, 1.0f / distance);
    child->fn_1_13264(&direction);
}

/* Enable player input when the start event arrives. */
void PlayerPiece::fn_1_276C(EventMessage &)
{
    inputEnabled = 1;
}

/* Disable player input when the finish event arrives. */
void PlayerPiece::fn_1_2778(EventMessage &)
{
    inputEnabled = 0;
}

/* Apply the two rolling inputs and wrap the model angles. */
void PlayerPiece::fn_1_2784(f32 horizontalDirection, f32 verticalDirection)
{
    this->horizontalStick = horizontalDirection;
    this->verticalStick = verticalDirection;
    this->horizontalAngle = this->horizontalAngle + (8.0f * horizontalDirection);
    this->verticalAngle = this->verticalAngle + (8.0f * verticalDirection);
    this->horizontalAngle = (f32) fmod((f64) this->horizontalAngle, 360.0);
    this->verticalAngle = (f32) fmod((f64) this->verticalAngle, 360.0);
}

/* Turn the piece toward its forward alignment and report whether it is still turning. */
bool PlayerPiece::fn_1_284C()
{
    Vec direction;

    direction.x = matrix[0][2];
    direction.y = matrix[1][2];
    direction.z = matrix[2][2];
    if (direction.z >= 0.99f)
        return false;

    if (direction.x > 0.0f)
    {
        fn_1_2784(1.0f, -1.0f);
    }
    else
    {
        fn_1_2784(-1.0f, 1.0f);
    }
    return true;
}

/* Choose automatic steering or pad input for the playing piece. */
void PlayerPiece::fn_1_29E0()
{
    f32 mainStick;
    f32 subStick;

    fn_1_2784(0.0f, 0.0f);

    if (inputEnabled == 0)
    {
        if (!fn_1_1E98() || !fn_1_284C())
            box.get()->fn_1_9EE4();
    }
    else
    {
        if (player.get()->isComputer())
        {
            fn_1_3AAC();
            return;
        }
        subStick = player.get()->subStickY();
        mainStick = player.get()->stickY();
        fn_1_2784(mainStick, subStick);
    }
}

/* Release the playing piece models, callbacks and owned game objects. */
PlayerPiece::~PlayerPiece()
{
}
