/* Steers a computer player through one of four queued routes. */
#include "REL/framework/frame_time.h"
#include "REL/framework/p_info.h"
#include "REL/m610DLL/module_types.h"
#include "REL/m610DLL/playing_piece.h"
#include <queue>
#include <memory>
#include "dolphin/mtx.h"

#include "REL/framework/random.h"

/* Left-hand route with seven steering points. */
const Vec lbl_1_rodata_1B8[7] = {
    { -901.5326f, 0.0f, -330.12738f },  { -839.9606f, 0.0f, -1055.0424f },
    { -506.56854f, 0.0f, -1129.1705f }, { -495.89764f, 0.0f, -1723.9176f },
    { -781.3999f, 0.0f, -2617.1108f },  { -291.97675f, 0.0f, -3039.562f },
    { 0.0f, 0.0f, -3900.0f }
};
/* Left-hand route with eight steering points. */
const Vec lbl_1_rodata_20C[8] = {
    { -796.8695f, 0.0f, -5.308167f },   { -871.9729f, 0.0f, -1065.4553f },
    { -1172.8733f, 0.0f, -1201.5145f }, { -1145.5337f, 0.0f, -1712.6956f },
    { -948.2461f, 0.0f, -2847.5063f },  { -418.7235f, 0.0f, -3012.4016f },
    { -267.25037f, 0.0f, -3539.5771f }, { 0.0f, 0.0f, -3900.0f }
};
/* Right-hand route with eight steering points. */
const Vec lbl_1_rodata_26C[8] = {
    { 796.8695f, 0.0f, -5.308167f },   { 871.9729f, 0.0f, -1065.4553f },
    { 1172.8733f, 0.0f, -1201.5145f }, { 1145.5337f, 0.0f, -1712.6956f },
    { 948.2461f, 0.0f, -2847.5063f },  { 418.7235f, 0.0f, -3012.4016f },
    { 267.25037f, 0.0f, -3539.5771f }, { 0.0f, 0.0f, -3900.0f }
};
/* Right-hand route with seven steering points. */
const Vec lbl_1_rodata_2CC[7] = {
    { 901.5326f, 0.0f, -330.12738f },  { 839.9606f, 0.0f, -1055.0424f },
    { 506.56854f, 0.0f, -1129.1705f }, { 495.89764f, 0.0f, -1723.9176f },
    { 781.3999f, 0.0f, -2617.1108f },  { 291.97675f, 0.0f, -3039.562f },
    { 0.0f, 0.0f, -3900.0f }
};

/* Vector operations used while steering toward the next route point. */
struct RouteVector : Vec
{
    /* Return the vector length. */
    float magnitude() const
    {
        return PSVECMag(this);
    }
    /* Return the dot product with another direction. */
    float dot(const Vec &other) const
    {
        return PSVECDotProduct(this, &other);
    }
    /* Scale this direction in place. */
    void scale(float factor)
    {
        PSVECScale(this, this, factor);
    }
};

/* The route a player follows and the timing of its two steering moods. */
class PathFollower
{
  public:
    /* Remaining route waypoints, consumed in order. */
    std::queue<const Vec *> points;
    /* Playing piece whose input this follower supplies. */
    PlayerPiece *actor;
    /* Frames remaining in the current steering mode. */
    int remainingFrames;
    /* Selects which steering duration weight is used. */
    int motionMode;
    /* Player difficulty used to select steering weights. */
    int difficulty;
    /* Weights used to choose the durations of the two steering modes. */
    float secondaryTimeWeight;
    float primaryTimeWeight;

    PathFollower(int difficulty, int playerNumber, PlayerPiece *owner);
    int steer(float angle);
    void update();
    /* Set the duration of the current steering mode. */
    void setFrames(int frames)
    {
        remainingFrames = frames;
    }
};

/* Select a player route and initialize its steering durations. */
PathFollower::PathFollower(int difficulty, int playerNumber, PlayerPiece *owner)
{
    int index0;
    int index1;
    int index2;
    int index3;

    actor = owner;
    remainingFrames = 0;
    this->difficulty = difficulty;
    motionMode = 1;
    secondaryTimeWeight = (float) (difficulty + 1) / 4.0f;
    primaryTimeWeight = 1.0f - secondaryTimeWeight;
    setFrames((int) (1.0f + secondsToFrames<float>(primaryTimeWeight * RandomFloat(1.0f))));

    if (playerNumber == 0)
    {
        for (index0 = 0; index0 < 7; ++index0)
        {
            points.push(&lbl_1_rodata_1B8[index0]);
        }
    }
    if (playerNumber == 1)
    {
        for (index1 = 0; index1 < 8; ++index1)
        {
            points.push(&lbl_1_rodata_20C[index1]);
        }
    }
    if (playerNumber == 2)
    {
        for (index2 = 0; index2 < 8; ++index2)
        {
            points.push(&lbl_1_rodata_26C[index2]);
        }
    }
    if (playerNumber == 3)
    {
        for (index3 = 0; index3 < 7; ++index3)
        {
            points.push(&lbl_1_rodata_2CC[index3]);
        }
    }
}

/* Create the player's route on first use, then follow it. */
void PlayerPiece::fn_1_3AAC()
{
    if (this->follower.get() == 0)
    {
        this->follower.reset(
            new PathFollower(this->playerReference()->difficulty(), this->playerNumber(), this));
    }
    this->follower->update();
}

/* Turn towards the next point: 0 when it is reached, 1 when it is ahead, 2 after turning. */
int PathFollower::steer(float steeringScale)
{
    RouteVector direction;
    RouteVector normal;
    RouteVector position;
    RouteVector displacement;
    float distance;
    float directionDot;
    float positionDot;
    float nextPointDot;

    actor->fn_1_1F10(&position);
    Vec nextPoint = *points.front();
    PSVECSubtract(&nextPoint, &position, &displacement);
    distance = displacement.magnitude();
    if (distance <= 20.0f)
        return 0;

    displacement.scale(1.0f / distance);
    actor->fn_1_1F2C(&direction);
    directionDot = direction.dot(displacement);
    if (directionDot > 0.99f)
    {
        actor->fn_1_2784(steeringScale, steeringScale);
        return 1;
    }

    actor->fn_1_1F48(&normal);
    positionDot = normal.dot(position);
    nextPointDot = normal.dot(nextPoint);
    if (nextPointDot > positionDot)
        actor->fn_1_2784(steeringScale, -steeringScale);
    else
        actor->fn_1_2784(-steeringScale, steeringScale);
    return 2;
}

/* Count down the steering mood, then steer towards the next point and drop it once reached. */
void PathFollower::update()
{
    static const float steeringScales[4] = { 0.6f, 0.7f, 0.8f, 1.0f };
    unsigned char reloadMotion = 0;
    float steeringScale;
    if (remainingFrames > 0)
    {
        if (--remainingFrames == 0)
        {
            reloadMotion = 1;
        }
    }
    if (reloadMotion != 0)
    {
        switch (motionMode)
        {
        case 0:
        {
            int steeringFrames =
                (int) (1.0f + secondsToFrames<float>(primaryTimeWeight * RandomFloat(4.0f, 7.0f)));
            remainingFrames = steeringFrames;
            motionMode = 1;
            break;
        }
        case 1:
        {
            int steeringFrames = (int) (1.0f + secondsToFrames<float>(secondaryTimeWeight *
                                                                      RandomFloat(7.0f, 10.0f)));
            remainingFrames = steeringFrames;
            motionMode = 0;
            break;
        }
        }
    }
    if (!points.empty())
    {
        if (motionMode == 0)
        {
            steeringScale = steeringScales[difficulty];
        }
        else
        {
            steeringScale = 0.23f;
        }
        if (steer(steeringScale) == 0)
        {
            points.pop();
        }
    }
}
