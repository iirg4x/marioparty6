#ifndef MD_MINI_WINDOW_STATE_H
#define MD_MINI_WINDOW_STATE_H

#include "dolphin/types.h"

/* Consumed signed-halfword states at offsets 0 and 2. The retail label covers
 * the first state; this shared view does not claim the original array bound. */
extern s16 lbl_1_data_82C[];

/* Three consumed signed words at offsets 0, 4 and 8 of the 12-byte retail
 * object. This is a recovered layout view, not an original declaration. */
extern s32 lbl_1_data_830[3];

#endif
