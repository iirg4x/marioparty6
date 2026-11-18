#ifndef REL_GAME_GRAVITY_H
#define REL_GAME_GRAVITY_H

// Gravity acceleration in centimeters per second squared.
static float gravityAcceleration = 980.665f;
// Gravity acceleration in centimeters per frame squared at 60 frames per second.
static float gravityPerFrame = (1.0 / 60.0) * ((1.0 / 60.0) * gravityAcceleration);

#endif
