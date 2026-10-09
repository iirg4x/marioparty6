/* Shared object, model, player, light, and minigame state for Asteroad Rage. */
#include "REL/m659/player.h"
#include "REL/m659/typed-context.h"

/* Global state shared by the Asteroad Rage player, obstacle, camera, and light systems. */
OMOBJ *lbl_1_bss_4C; /* Obstacle manager object, whose data holds the obstacle course. */
HU3D_MODELID lbl_1_bss_3C[8]; /* Eight course-edge model IDs arranged across the lanes; six link
                               * to two created models. */
OMOBJ *lbl_1_bss_2C[4]; /* Player objects indexed by split-screen side. */
HU3D_MODELID lbl_1_bss_28; /* Shared finish-grid model shown during the result scene. */
M659Work lbl_1_bss_4; /* Shared minigame, camera, and result-sequence state. */
HU3D_LIGHTID lbl_1_bss_0; /* Light installed for the result presentation. */
