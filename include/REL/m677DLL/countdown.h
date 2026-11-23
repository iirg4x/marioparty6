// Advances frame countdowns for events and the rope release sequence.

#ifndef REL_SEER_TERROR_COUNTDOWN_H
#define REL_SEER_TERROR_COUNTDOWN_H

// Advances the countdown and reports the frame on which it expires.
inline bool updateCountdown(int &countdown)
{
    return countdown > 0 && --countdown == 0;
}

#endif
