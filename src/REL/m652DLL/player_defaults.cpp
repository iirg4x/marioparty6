#include "REL/m652DLL/time_conversion.h"
#include "REL/m652DLL/map_resource.h"
extern "C" {

unsigned int lbl_1_bss_78 = fn_1_2178(5);
unsigned int lbl_1_bss_7C = secondsToFrames<long>(30);
float lbl_1_bss_80 = framesToSeconds(framesToSeconds(980.665));
}

MapResource::MapResource(OwnedModel *value)
{
    this->value = value;
}
