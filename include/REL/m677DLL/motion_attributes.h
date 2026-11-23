// Motion-attribute masks used by the Seer Terror models.

#ifndef REL_SEER_TERROR_MOTION_ATTRIBUTES_H
#define REL_SEER_TERROR_MOTION_ATTRIBUTES_H

enum {
    // Wrap the main HSF motion at its playback endpoints.
    loopMotion = 0x40000001,
    // Hold the main HSF motion at its current playback time.
    pauseMotion = 0x40000002,
    // Wrap the overlay HSF motion at its playback endpoints.
    loopOverlayMotion = 0x40000200
};

#endif
