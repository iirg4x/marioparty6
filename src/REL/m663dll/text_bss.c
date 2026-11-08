/* Module-wide work: camera, light, shadow, projection, particles, models, players, input,
 * score sprites and background controls. */
#include "REL/m663dll/transition.h"
#include <math.h>

#include "dolphin/types.h"
#include "REL/m663dll/models.h"
#include "REL/m663dll/tutorial.h"
#include "REL/m663dll/input.h"
#include "REL/m663dll/player.h"
#include "game/process.h"
#include "REL/m663dll/camera.h"
#include "REL/m663dll/lighting.h"
#include "REL/m663dll/projection.h"
#include "REL/m663dll/shadow.h"
#include "REL/m663dll/particles.h"
#include "REL/m663dll/score.h"
#include "REL/m663dll/actors.h"
#include "REL/m663dll/motion.h"

u16 lbl_1_bss_15A8;
HUPROCESS *lbl_1_bss_15A0[2];
M663CameraState lbl_1_bss_1428;
M663LightState lbl_1_bss_13EC;
M663ShadowState lbl_1_bss_139C;
M663ProjectionState lbl_1_bss_1378;
M663ParticleState lbl_1_bss_1368;
M663ModelWork lbl_1_bss_750[86];
M663PlayerMotionWork lbl_1_bss_688[2];
M663PlayerWork lbl_1_bss_4F0[2];
M663ModuleInput lbl_1_bss_424;
M663PlayerActorMap lbl_1_bss_418;
M663TutorialWork lbl_1_bss_378[5];
M663SpriteGroup lbl_1_bss_350[2];
M663ScoreAttribute lbl_1_bss_120[14];
M663TransitionControl lbl_1_bss_0[6];
