#include "REL/m654DLL/time_conversion.h"
template<class Time> Time secondsToFrames(Time);


struct BssExtent180 {
    int consumed_word;
    unsigned char unknown04[4]; /* No code names this field. */
    BssExtent180() : consumed_word((int)secondsToFrames(3.5)) {}
};
extern "C" {
int lbl_1_bss_38 = secondsToFrames<long>(2);
int lbl_1_bss_3C = (int)secondsToFrames(3.5);
BssExtent180 lbl_1_bss_40;
}

// Same-game framework time conversion.
template<class Time> Time secondsToFrames(Time seconds) { return 60 * seconds; }
