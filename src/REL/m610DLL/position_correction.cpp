/* Apply position corrections while retaining the previous position for Verlet integration. */
#include "dolphin/mtx.h"
#include "REL/m610DLL/position_state.h"

/* Apply the opposite correction to the previous position when replacing the current position. */
void PositionCorrectionState::fn_1_12460(const ProjectionVector *position)
{
    ProjectionVector &currentPosition = this->position;
    this->previous += currentPosition.difference(*position);
    currentPosition = *position;
}

/* Move the current position toward the previous position by a fraction of their displacement. */
void PositionCorrectionState::fn_1_124F0(float fraction)
{
    ProjectionVector &currentPosition = this->position;
    currentPosition -= fn_1_12584() * fraction;
}

/* Return the displacement from the previous position to the current position. */
ProjectionVector PositionCorrectionState::fn_1_12584() const
{
    const ProjectionVector &position = this->position;
    return position - this->previous;
}

/* Return the displacement divided by the supplied amount. */
ProjectionVector PositionCorrectionState::fn_1_125F8(float amount) const
{
    return fn_1_12584() / amount;
}

/* Save the current position and clear displacement before the next motion update. */
void PositionCorrectionState::fn_1_126F0()
{
    const Vec *position = &this->position;
    this->previous.x = position->x;
    this->previous.y = position->y;
    this->previous.z = position->z;
    this->displacement.x = 0.0f;
    this->displacement.y = 0.0f;
    this->displacement.z = 0.0f;
}

/* Apply the stored displacement when this position state advances. */
void PositionCorrectionState::fn_1_12750()
{
    Vec *position = &this->position;
    PSVECAdd(position, &this->displacement, position);
}

/* Store the scaled frame displacement and retain the current position for the next update. */
void PositionCorrectionState::fn_1_12794()
{
    ProjectionVector &position = this->position;
    this->displacement = position.difference(this->previous);
    PSVECScale(&this->displacement, &this->displacement, 0.9999f);
    this->previous = position;
}
