// Animates a Bubble falls model and draws its light and shadow.

#ifndef REL_SEER_TERROR_BUBBLE_FALLS_MODEL_H
#define REL_SEER_TERROR_BUBBLE_FALLS_MODEL_H

#include "REL/framework/game_object2.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/model.h"

// Animates a falling model with a point light and a ground shadow.
class BubbleFallsModel : public RenderObject {
public:
    BubbleFallsModel(const OwnedModel &source, const Vec &startPosition,
                      float angle, bool enabled);
    virtual ~BubbleFallsModel();
    virtual void update();
    virtual void draw(Mtx *matrix);
    void startFallLight();
    void waitForLanding();
    void waitBeforeShrinking();
    void shrinkBubble();

    void configureShadowDraw(Mtx view);
    void drawShadowDisc();

    // Selected update function for the fall and landing sequence.
    typedef void (BubbleFallsModel::*State)();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    unsigned char enabled;
    // Point-light handle, or -1 before allocation.
    short lightId;
    OwnedModel model;
    Vec position;
    Vec velocity;
    // Interpolates the model scale during the sequence.
    FrameInterpolator<float> modelScale;
    float phase;
    // Frames remaining in a timed playback state.
    int timer;
};

int countBubbleFallsModels();
void moveFallingBubble(BubbleFallsModel *owner);
void burstBubbleParticles(BubbleFallsModel *owner, int particleCount);

#endif
