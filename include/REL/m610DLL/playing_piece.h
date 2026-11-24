#ifndef REL_M610_PLAYING_PIECE_H
#define REL_M610_PLAYING_PIECE_H
#include "REL/framework/category_sphere.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/octtree_man.h"
#include "REL/framework/owned_pointer.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/p_info.h"
struct TriangleCollision;
struct RigidBox;
class PathFollower;
struct ParticleEmitterState;

/* Owns and updates one player playing piece, input and attached particles. */
struct PlayerPiece : GameObject
{
    bool inputEnabled;      // Channels 2/3 enable/disable the input branch in fn_1_29E0.
    bool crossedDepthLimit; // Latched when the box position passes z = -3800.0f.
    /* Number of scripted movement steps still to apply. */
    s32 remainingSteps;
    /* Transform applied to the six surrounding models. */
    Mtx matrix;
    /* Horizontal and vertical stick input used by the rotation update. */
    float horizontalStick;
    float verticalStick;
    /* Accumulated angles used to rotate the playing piece. */
    f32 horizontalAngle;
    f32 verticalAngle;
    /* Registrations for the six surrounding models. */
    RegisteredValue entries[6];
    /* Source model whose transform is updated with the playing piece. */
    std::auto_ptr<OwnedModel> animation;
    /* Six surrounding models, with matching collision registrations. */
    std::auto_ptr<OwnedModel> models[6];
    /* Collision tree shared by the piece and its box solver. */
    OwnedPointer<TriangleCollision> collision;
    /* Configuration and controller state for this player. */
    OwnedPointer<PlayerInfo> player;
    /* Model used alongside the playing piece and its surrounding models. */
    std::auto_ptr<OwnedModel> auxiliaryModel;
    /* Corner solver used to derive the piece transform. */
    OwnedPointer<RigidBox> box;
    /* Event callbacks that change input and movement state. */
    EventHandlerManager callbacks;
    /* Automatic steering for a computer controlled player. */
    std::auto_ptr<PathFollower> follower;
    /* Particle emitter updated with the piece position. */
    OwnedPointer<ParticleEmitterState> child;

    PlayerPiece(OwnedModel *animation, OwnedModel *modelSource, TriangleCollision *collision,
                PlayerInfo *player, ParticleEmitterState *child);
    virtual ~PlayerPiece();
    virtual void update();
    void fn_1_1F10(Vec *position) const;
    void fn_1_1F2C(Vec *direction) const;
    void fn_1_1F48(Vec *normal) const;
    void fn_1_2784(f32, f32);
    void fn_1_3AAC();
    /* Return this playing piece player configuration. */
    PlayerInfo *playerReference() const
    {
        return player.get();
    }
    /* Read the player number from this playing piece configuration. */
    int playerNumber() const
    {
        const PlayerInfo *reference = playerReference();
        return reference->playerNo;
    }
    void fn_1_1F70();
    void fn_1_1FAC(const Vec *);
    bool fn_1_1E98();
    void fn_1_20D8();
    void fn_1_21C4();
    bool fn_1_284C();
    void fn_1_29E0();
    void fn_1_24A8();
    void fn_1_276C(EventMessage &);
    void fn_1_2778(EventMessage &);
};

extern "C"
{
    extern PlayerPiece *lbl_1_bss_54[4];
}

#endif
