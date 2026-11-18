// Lengths in world units. A coordinate written with it is set by the static initialiser.
#ifndef REL_FRAMEWORK_WORLD_UNITS_H
#define REL_FRAMEWORK_WORLD_UNITS_H

// Returns the length in world units, where one unit is one centimeter.
template <class Length> inline Length cmToWorld(Length centimeters)
{
    return centimeters;
}

#endif
