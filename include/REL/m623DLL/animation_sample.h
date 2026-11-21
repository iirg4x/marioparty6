#ifndef REL_M623DLL_ANIMATION_SAMPLE_H
#define REL_M623DLL_ANIMATION_SAMPLE_H

// Stores one vertex across the animated path strip.
struct AnimationSample {
    // Position across the strip.
    float x;
    // Depth offset from the path segment.
    float zOffset;
    // Vertex alpha.
    int opacity;
    // Texture coordinate across the strip.
    float texcoord;
};

#endif
