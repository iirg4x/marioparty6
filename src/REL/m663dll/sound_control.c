/* Pan, pitch and volume control for the module's sound effect slots. */
#include "dolphin/types.h"
#include "game/audio.h"
#include "REL/m663dll/input.h"

/* Sets the pan of a player's sound effect when its slot is active and its status is 2. */
void M663SoundPanSet(s16 playerIndex, s16 pan) {
    if (lbl_1_bss_424.soundActive[playerIndex] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_424.soundHandles[playerIndex]) == 2) {
            HuAudFXPanning(lbl_1_bss_424.soundHandles[playerIndex], pan);
        }
    }
}

/* Sets the pitch of a player's sound effect when its slot is active and its status is 2. */
void M663SoundPitchSet(s16 playerIndex, s16 pitch) {
    if (lbl_1_bss_424.soundActive[playerIndex] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_424.soundHandles[playerIndex]) == 2) {
            HuAudFXPitchSet(lbl_1_bss_424.soundHandles[playerIndex], pitch);
        }
    }
}

/* Sets the volume of a player's sound effect when its slot is active and its status is 2. */
void M663SoundVolumeSet(s16 playerIndex, s16 volume) {
    if (lbl_1_bss_424.soundActive[playerIndex] == 1) {
        if (HuAudFXStatusGet(lbl_1_bss_424.soundHandles[playerIndex]) == 2) {
            HuAudFXVolSet(lbl_1_bss_424.soundHandles[playerIndex], volume);
        }
    }
}
