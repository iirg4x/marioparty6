/* Cashapult setup, play, scoring, and character animations. */
#include "REL/m642dll/work.h"
#include "math.h"
#include "msm_stream.h"
#include "msm_se.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "dolphin/types.h"
#include "game/hu3d.h"

#define M642_EFFECT_INACTIVE_MASK 0xFFFFFFFE
#define M642_SCENE_PLAY_DONE 1
#define M642_SCENE_RESULTS_DONE 2
#define MSM_SE_CASHAPULT_LAUNCHER_LEFT 1942
#define MSM_SE_CASHAPULT_LAUNCHER_RIGHT 1943
#define MSM_SE_CASHAPULT_JUMP_LEFT 1944
#define MSM_SE_CASHAPULT_JUMP_RIGHT 1945
#define MSM_SE_CASHAPULT_FLIGHT_LEFT 1946
#define MSM_SE_CASHAPULT_FLIGHT_RIGHT 1947
#define MSM_SE_CASHAPULT_LAND_LEFT 1948
#define MSM_SE_CASHAPULT_LAND_RIGHT 1949
#define MSM_SE_CASHAPULT_INTRO 1950

void fn_1_318(void);
void fn_1_31C(void);
M642SceneWork *lbl_1_bss_2C; /* Scene objects, score displays and round progression. */
M642PlayerWork *lbl_1_bss_1C[4]; /* Character movement and CPU steering for each player. */
M642GridWork *lbl_1_bss_18; /* Coin grids and the two launcher slots on each team. */
M642RoundControl *lbl_1_bss_14; /* Split-screen cameras and the players they follow. */
void *lbl_1_bss_10;
M642EffectWork *lbl_1_bss_C;
void *lbl_1_bss_8;
/* Converts a direction in the XZ plane to its heading in degrees. */
f32 fn_1_320(HuVecF *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / 3.141592653589793;
}

/* Smoothly turns player movement toward a heading along the shorter route. */
f32 fn_1_378(f32 start, f32 end, f32 blend)
{
    f32 delta;
    f32 result;

    delta = fmod(end - start, 360.0);
    if (delta < 0.0f) {
        delta += 360.0f;
    }
    if (delta > 180.0f) {
        delta -= 360.0f;
    }
    result = fmod(start + blend * delta, 360.0);
    if (result < 0.0f) {
        result += 360.0f;
    }
    return result;
}

/* Assigns two players to each Cashapult team, falling back to alternating teams during setup. */
void fn_1_47C(void *sceneWork)
{
    s32 teamCounts[4];
    s32 playerIndex;
    s32 team;

    if (lbl_1_bss_2C != NULL) {
        playerIndex = 0;
        while (playerIndex < 4) {
            teamCounts[playerIndex] = 0;
            playerIndex++;
        }
        playerIndex = 0;
        while (playerIndex < 4) {
            team = GwPlayerConf[playerIndex].grpNo;
            lbl_1_bss_2C->playerGroups[playerIndex] = team;
            teamCounts[team]++;
            playerIndex++;
        }
        if ((teamCounts[0] != 2) || (teamCounts[1] != 2) ||
            (teamCounts[2] != 0) || (teamCounts[3] != 0)) {
            playerIndex = 0;
            while (playerIndex < 4) {
                lbl_1_bss_2C->playerGroups[playerIndex] = playerIndex % 2;
                playerIndex++;
            }
        }
    }
}

/* Reads a player's assigned team during setup; no value is supplied before the scene exists. */
s32 fn_1_594(M642PlayerWork *playerWork)
{
    if (lbl_1_bss_2C == NULL) {
        return;
    }
    return lbl_1_bss_2C->playerGroups[playerWork->playerNo];
}

/* Creates the Cashapult scene object and stores its scene work pointer. */
void fn_1_5CC(void)
{
    lbl_1_bss_2C = fn_1_A0(100, 372, fn_1_31CC);
}

/* Checks whether play has finished when scene work exists; the result is discarded. */
void fn_1_608(void)
{
    if (lbl_1_bss_2C != NULL && lbl_1_bss_2C->state == M642_SCENE_PLAY_DONE) {
    }
}

/* The sequence polls this after play to wait for every player's final motion to finish. */
s32 fn_1_64C(void)
{
    if (lbl_1_bss_2C == NULL) {
        return 0;
    }
    return lbl_1_bss_2C->state == M642_SCENE_PLAY_DONE;
}

/* The sequence polls this during results to wait until each player has received the team coins. */
s32 fn_1_698(void)
{
    if (lbl_1_bss_2C == NULL) {
        return 0;
    }
    return lbl_1_bss_2C->state == M642_SCENE_RESULTS_DONE;
}

typedef struct M642ScoreSpriteConfig_s {
    s32 assetIndex; /* Index into the scene's loaded sprite animations. */
    Vec position; /* Screen-space offset from the score group. */
} M642ScoreSpriteConfig;
typedef struct M642ScoreConfig_s {
    Vec position; /* Screen-space origin for this team's score. */
    Vec digitOffset; /* Offset from the origin to the numeric score. */
    u8 red, green, blue; /* Score-box color channels. */
    u8 reservedColorByte; /* Stored with the color but not read by setup. */
    s32 maxDigits; /* Maximum digits shown by the score display. */
    s32 spriteCount; /* Number of indicator sprites created for this team. */
    M642ScoreSpriteConfig sprites[3]; /* Indicator asset and position entries. */
} M642ScoreConfig;

M642ScoreConfig lbl_1_data_30[2] = {
    {
        {59.0f, 420.0f, 0.0f}, {8.0f, 0.0f, 0.0f},
        250, 0, 30, 255, 2, 2,
        {
            {0, {-28.0f, 0.0f, 0.0f}},
            {1, {-8.0f, 0.0f, 0.0f}},
            {0, {0.0f, 0.0f, 0.0f}}
        }
    },
    {
        {517.0f, 420.0f, 0.0f}, {8.0f, 0.0f, 0.0f},
        0, 50, 250, 255, 2, 2,
        {
            {0, {-28.0f, 0.0f, 0.0f}},
            {1, {-8.0f, 0.0f, 0.0f}},
            {0, {0.0f, 0.0f, 0.0f}}
        }
    }
};

unsigned int lbl_1_data_D8[7] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1), DATANUM(DATA_mariomot, 2),
    DATANUM(DATA_mariomot, 3), DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 38)
};
s32 lbl_1_data_F4[4] = {5, 11, 15, -1};
char lbl_1_data_104[] = "642seesaw-P1st";
char lbl_1_data_113[] = "642seesaw-P2st";
char *lbl_1_data_124[2] = {lbl_1_data_104, lbl_1_data_113};
u32 lbl_1_data_12C[6][20] = {
    {11, 0, 2, 0, 4, 0, 12, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {13, 0, 6, 0, 8, 0, 7, 10, 0, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 3, 9, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {6, 0, 7, 10, 0, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {2, 0, 3, 9, 0, 5, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {6, 0, 14, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

Vec lbl_1_data_30C = {0.0f, 0.0f, 2000.0f};
Vec lbl_1_data_318 = {0.0f, 0.0f, -1.0f};
GXColor lbl_1_data_324[2] = {{255, 255, 255, 0}, {242, 234, 255, 0}};
Vec lbl_1_data_32C = {0.0f, 700.0f, 0.0f};
Vec lbl_1_data_338 = {0.0f, 490.0f, 0.0f};
Vec lbl_1_data_344 = {0.0f, 0.0f, 2000.0f};
Vec lbl_1_data_350 = {0.0f, 0.0f, 2000.0f};
Vec lbl_1_data_35C = {0.0f, 0.0f, 1500.0f};
Vec lbl_1_data_368 = {0.0f, 1.0f, 0.0f};

typedef struct M642TrackKeyView {
    s32 frame; /* Animation frame; a negative frame ends the key list. */
    f32 values[3]; /* Channel values sampled from this key. */
} M642TrackKeyView;

typedef struct M642TrackView {
    s16 kind, count, end; /* Interpolation mode, channel count and final active frame. */
    M642TrackKeyView *keys; /* Ordered keys followed by a negative-frame terminator. */
} M642TrackView;

typedef struct M642EffectRecord_s {
    u16 flags; /* Bit zero marks an effect that receives updates. */
    s16 frame, type, camera; /* Animation age, pool category and rendering camera mask. */
    HU3D_MODELID model; /* Reusable model shown while the effect is active. */
    s16 phase; /* Reset on activation; not advanced by this effect callback. */
    u8 unreadData[24]; /* Record bytes not accessed by the effect callbacks. */
    void (*update)(struct M642EffectRecord_s *); /* Called once each active effect tick. */
    M642TrackView **tracks; /* Three track slots; this effect samples scale from slot two. */
    Vec position; /* World-space origin copied when the effect starts. */
} M642EffectRecord;

M642TrackKeyView lbl_1_data_374[4] = {
    {0, {0.30000001192092896f, 0.30000001192092896f, 0.30000001192092896f}},
    {10, {1.0f, 1.0f, 1.0f}},
    {30, {0.0f, 0.0f, 0.0f}},
    {-1, {0.0f, 0.0f, 0.0f}}
};
M642TrackView lbl_1_data_3B4 = {0, 3, 30, lbl_1_data_374};
M642TrackView *lbl_1_data_3C0[3] = {NULL, NULL, &lbl_1_data_3B4};

extern void fn_1_31C(void);
extern void fn_1_318(void);

MGPLAYER_MODE_FUNC lbl_1_data_3CC[MGPLAYER_MODE_MAX] = {
    NULL,
    (MGPLAYER_MODE_FUNC)fn_1_31C,
    (MGPLAYER_MODE_FUNC)fn_1_318
};
extern void fn_1_914(s32 player, s32 visible);

/* Creates one team's score box, digits, and hidden sprite indicators during scoreboard setup. */
void fn_1_6E4(M642ScoreConfig *config)
{
    s32 box, scoreIndex, spriteGroup, spriteIndex, assetIndex, sprite;
    MGSCORE *score;

    box = MgScoreBoxCreate(88, 40);
    MgScoreBoxPosSet(box, config->position.x, config->position.y);
    MgScoreBoxColorSet(box, config->red, config->green, config->blue);
    score = MgScoreCreate(DATANUM(DATA_mgconst, 49), -1, 1);
    MgScoreMaxDigitSet(score, config->maxDigits);
    MgScorePosSet(score, config->position.x + config->digitOffset.x,
                  config->position.y + config->digitOffset.y);
    MgScoreColorSet(score, 255, 216, 21);
    scoreIndex = ((M642SceneWork *)lbl_1_bss_2C)->scoreCount;
    ((M642SceneWork *)lbl_1_bss_2C)->players[scoreIndex].box = box;
    ((M642SceneWork *)lbl_1_bss_2C)->players[scoreIndex].display = score;
    ((M642SceneWork *)lbl_1_bss_2C)->players[scoreIndex].spriteCount = config->spriteCount;
    spriteGroup = HuSprGrpCreate(config->spriteCount);
    HuSprGrpPosSet(spriteGroup, config->position.x, config->position.y);
    for (spriteIndex = 0; spriteIndex < config->spriteCount; spriteIndex++) {
        assetIndex = config->sprites[spriteIndex].assetIndex;
        sprite = HuSprCreate(((M642SceneWork *) lbl_1_bss_2C)->spriteAssets[assetIndex].animation,
                             10, 0);
        HuSprGrpMemberSet(spriteGroup, spriteIndex, sprite);
        HuSprAttrSet(spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(spriteGroup, spriteIndex, config->sprites[spriteIndex].position.x,
                    config->sprites[spriteIndex].position.y);
    }
    ((M642SceneWork *)lbl_1_bss_2C)->players[scoreIndex].spriteGroup = spriteGroup;
    ((M642SceneWork *)lbl_1_bss_2C)->scoreCount++;
    ((M642SceneWork *)lbl_1_bss_2C)->players[scoreIndex].value = 0;
    MgScoreValueSet(score, 0);
    fn_1_914(scoreIndex, 0);
}

/* Shows or hides a team's score box, number, and sprite indicators during play. */
void fn_1_914(s32 team, s32 visible)
{
    M642ScoreRecord *score;
    s32 spriteIndex;
    score = &((M642SceneWork *)lbl_1_bss_2C)->players[team];
    if (visible == 1) {
        MgScoreBoxDispSet(score->box, 1);
        MgScoreDispOn(score->display);
        for (spriteIndex = 0; spriteIndex < score->spriteCount; spriteIndex++) {
            HuSprAttrReset(score->spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
        }
    } else {
        MgScoreBoxDispSet(score->box, 0);
        MgScoreDispOff(score->display);
        for (spriteIndex = 0; spriteIndex < score->spriteCount; spriteIndex++) {
            HuSprAttrSet(score->spriteGroup, spriteIndex, HUSPR_ATTR_DISPOFF);
        }
    }
}

/* Adds collected coins to a team's displayed score. */
void fn_1_9FC(s32 team, s32 coins)
{
    M642ScoreRecord *score;
    score = &((M642SceneWork *)lbl_1_bss_2C)->players[team];
    score->value += coins;
    MgScoreValueSet(score->display, score->value);
}

/* Returns the current score for a team when the results screen is prepared. */
s32 fn_1_A60(s32 team)
{
    M642ScoreRecord *record = &((M642SceneWork *)lbl_1_bss_2C)->players[team];
    return record->value;
}

/* Called during player setup to place the actor at an open team launcher. */
void fn_1_A90(M642PlayerWork *work)
{
    HSF_OBJECT *node = NULL;
    s32 team, slot;
    Vec position;
    if (lbl_1_bss_18 != NULL) {
        team = work->team;
        slot = frandmod(2);
        if (lbl_1_bss_18->players[team][slot]) {
            slot = 1 - slot;
        }
        lbl_1_bss_18->players[team][slot] = work;
        node = Hu3DModelObjPtrGet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], lbl_1_data_124[slot]);
        position.x = node->mesh.base.pos.x;
        position.y = node->mesh.base.pos.y;
        position.z = node->mesh.base.pos.z;
        MgActorPosSet(work->player->actor, &position);
        work->position = position;
        work->slot = slot;
        work->baseRotation = node->mesh.base.rot.y;
        MgActorRotYSet(work->player->actor, work->baseRotation);
        work->player->actor->gravity = 0.0f;
        lbl_1_bss_18->count[team]++;
    }
}

/* Tests the turn's launcher slot for round callbacks; no value is supplied without a grid. */
BOOL fn_1_C14(M642PlayerWork *player)
{
    if (lbl_1_bss_18 == NULL) {
        return;
    }
    if (lbl_1_bss_18->players[0][lbl_1_bss_18->active] == player) {
        return TRUE;
    }
    if (lbl_1_bss_18->players[1][lbl_1_bss_18->active] == player) {
        return TRUE;
    }
    return FALSE;
}

/* Returns a team's active player for grid collisions; without a grid, returns the team value as
 * a pointer. */
void *fn_1_CAC(s32 team)
{
    if (lbl_1_bss_18 != NULL) {
        if (lbl_1_bss_18->players[team][lbl_1_bss_18->active]) {
        }
        team = (s32) lbl_1_bss_18->players[team][lbl_1_bss_18->active];
    }
    return (void *)team;
}

/* Alternates the active teammate slot after each completed round. */
void fn_1_D2C(void)
{
    lbl_1_bss_18->active = (lbl_1_bss_18->active + 1) % 2;
}

typedef struct M667Pool M667Pool;
extern void *fn_1_86B0(M667Pool *pool);

/* Allocates and initializes one cell in a team's grid pool during grid setup. */
M642GridCell *fn_1_D6C(s32 team, M642GridCell *initial)
{
    M642GridCell *cell;
    if (lbl_1_bss_18 == NULL) {
        return;
    }
    cell = fn_1_86B0(lbl_1_bss_18->pools[team]);
    if (!cell) {
        return;
    }
    *cell = *initial;
    return cell;
}

typedef struct M667Pool M667Pool;
extern void *fn_1_8794(void *pool);
extern void *fn_1_87D0(void *pool, void *item);

/* Called by the grid update to test a team's player against its collectible cells. */
void fn_1_E48(s32 team, void *playerData)
{
    M642GridWork *grid;
    M667Pool *pool;
    M642GridCell *cell;
    M642PlayerWork *playerWork;
    MGACTOR *actor;
    COLBODY *collisionBody;
    Vec cellPosition, playerPosition, delta;
    float cellRadius, actorRadius, centerDistance, combinedRadius;

    grid = lbl_1_bss_18;
    pool = grid->pools[team];
    cell = fn_1_8794(pool);
    while (cell) {
        cell->flags &= ~2;
        cell = fn_1_87D0(pool, cell);
    }
    playerWork = playerData;
    actor = playerWork->player->actor;
    collisionBody = ColBodyGet(actor->no);
    actorRadius = collisionBody->param.radius;
    MgActorPosGet(actor, &playerPosition);
    playerPosition.y += 75.0f;
    cell = fn_1_8794(pool);
    while (cell) {
        if (!(cell->flags & 4) && cell->update) {
            cell->update((u8 *)cell);
        }
        if (!(cell->flags & 6)) {
            cellRadius = cell->radius;
            cellPosition = cell->position;
            PSVECSubtract(&playerPosition, &cellPosition, &delta);
            centerDistance = sqrtf(delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y));
            combinedRadius = cellRadius + actorRadius;
            if (fabs(centerDistance) < combinedRadius) {
                cell->flags |= 2;
                if (cell->collect) {
                    cell->collect(playerWork, cell);
                }
            }
        }
        cell = fn_1_87D0(pool, cell);
    }
}

/* Sets round control to its first active mode after the intro motion ends. */
void fn_1_105C(void)
{
    if (lbl_1_bss_14 != NULL) {
        lbl_1_bss_14->mode = 1;
    }
}

/* Sets round control to the results mode when the final sequence begins. */
void fn_1_108C(void)
{
    if (lbl_1_bss_14 != NULL) {
        lbl_1_bss_14->mode = 5;
    }
}

/* Cell-model update callback: applies its yaw and advances that yaw by five degrees. */
void fn_1_10BC(u8 *data)
{
    Hu3DModelRotSetV(((M642GridCell *)data)->model,
        &((M642GridCell *)data)->rotation);
    ((M642GridCell *)data)->rotation.y += 5.0f;
}

extern s32 fn_1_8F9C(Vec *position, s32 camera, float pan, float volume, s32 sound);
extern void fn_1_9FC(s32 player, s32 value);

/* Collects a cell during the grid update, plays its glow and sound, and adds one coin. */
void fn_1_1108(M642PlayerWork *player, M642GridCell *cell)
{
    s32 camera;
    s32 sound;
    float pan, volume;
    cell->flags |= 4;
    camera = cell->owner == 0 ? 1 : 2;
    CharEffectCoinGlowCreate(camera, &cell->position);
    sound = MSM_SE_CMN_08;
    pan = 0.0f;
    volume = 640.0f;
    fn_1_8F9C(&cell->position, camera, pan, volume, sound);
    Hu3DModelAttrSet(cell->model, HU3D_ATTR_DISPOFF);
    fn_1_9FC(player->team, 1);
}

extern s32 lbl_1_data_1578[5][9][9];
extern void fn_1_10BC(u8 *);
extern void fn_1_1108(M642PlayerWork *, M642GridCell *);
extern M642GridCell *fn_1_D6C(s32 player, M642GridCell *initial);

extern int abs(int);

extern s32 fn_1_1ABC(s32 player, s32 row, s32 column);

typedef struct M642Candidate_s {
    M642GridCell *cell; /* Uncollected coin considered as a CPU target. */
    float distance; /* Horizontal world-space distance from the player. */
    s16 row; /* Grid row saved when this candidate is selected. */
    s16 column; /* Grid column saved when this candidate is selected. */
} M642Candidate;

extern void fn_1_8870(M642TrackView **tracks, s32 frame, s32 trackIndex, f32 *values);
extern s32 fn_1_8814(M642TrackView **tracks, s32 frame);
extern void fn_1_3190(M642EffectRecord *data);

typedef struct M642EmptyCandidate_s {
    float distance; /* World-space distance from the player to this grid position. */
    s32 priority; /* Nearby uncollected-coin count; lower counts sort first. */
    float x; /* World-space horizontal coordinate of the empty grid position. */
    float y; /* World-space height of the empty grid position. */
} M642EmptyCandidate;

/* Builds up to twenty coins from the selected pattern for one team during grid setup. */
void fn_1_11E4(s32 team, s32 pattern)
{
    M642GridWork *grid = lbl_1_bss_18;
    M642GridCell initialCell = {0};
    MGACTOR_PARAM unusedActorParameters = {0};
    Vec position;
    s32 row = 0, column = 0, coinCount = 0;
    s32 camera, model;
    float columnWidth, rowHeight;

    camera = team == 0 ? 1 : 2;
    for (row = 0; row < 9; row++) {
        for (column = 0; column < 9; column++) {
            if (coinCount >= 20) {
                goto done;
            }
            if (lbl_1_data_1578[pattern][row][column]) {
                model = lbl_1_bss_2C->scene_objects[6]->mdlId[lbl_1_bss_18->cellModels + 1];
                initialCell.model = model;
                initialCell.collect = fn_1_1108;
                initialCell.update = fn_1_10BC;
                initialCell.owner = team;
                initialCell.height = 40.0f;
                initialCell.radius = 40.0f;
                lbl_1_bss_18->grid[team][row][column] = fn_1_D6C(team, &initialCell);
                columnWidth = 100.0f;
                rowHeight = 200.0f;
                position.x = 0.5f * columnWidth + (-450.0f + columnWidth * column);
                position.y = 5400.0f - rowHeight * row - 0.5f * rowHeight;
                position.z = 0.0f;
                lbl_1_bss_18->grid[team][row][column]->position = position;
                Hu3DModelPosSetV(model, &position);
                Hu3DModelCameraSet(model, camera);
                lbl_1_bss_18->cellModels++;
                coinCount++;
            }
        }
    }
done:
    coinCount = 0;
}

/* Effect-pool callback: shows and scales a smoke model, then hides it after its track ends. */
void fn_1_1564(M642EffectRecord *work)
{
    Vec scale;
    Vec position;
    s32 model, frame;
    model = work->model;
    frame = work->frame;
    if (frame == 0) {
        Hu3DModelCameraSet(model, (u16)work->camera);
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
        position = work->position;
        position.z += 400.0f;
        position.x *= 0.800000011920929f;
        Hu3DModelPosSetV(model, &position);
    }
    fn_1_8870(work->tracks, work->frame, 2, &scale.x);
    Hu3DModelScaleSet(model, scale.x, scale.y, scale.z);
    if (fn_1_8814(work->tracks, work->frame)) {
        fn_1_3190(work);
    }
}

/* CPU planners use the first row with available coins to choose a world-space steering target. */
BOOL fn_1_1660(s32 team, Vec *position, Vec *target, u32 flags)
{
    M642Candidate coinCandidates[9];
    Vec cellPosition;
    M642Candidate candidateSwap;
    s32 firstRow, candidateCount, row, column, candidateIndex, otherIndex, selectedIndex;
    M642GridCell *cell, *selectedCell;
    BOOL coinAvailable, selectionBlocked;
    float horizontalDistance;

    firstRow = (5400.0f - position->y) / 200.0f;
    firstRow = abs(firstRow);
    if (firstRow >= 9) return FALSE;
    candidateCount = 0;
    selectedCell = NULL;
    selectionBlocked = FALSE;
    for (row = firstRow; row < 9; row++) {
        for (column = 0; column < 9; column++) {
            cell = lbl_1_bss_18->grid[team][row][column];
            if (cell) {
                coinAvailable = (cell->flags & 4) == 0 ? TRUE : FALSE;
                if ((flags & 3) && lbl_1_bss_2C->selection[0] == row &&
                    lbl_1_bss_2C->selection[1] == column) {
                    coinAvailable = FALSE;
                    selectionBlocked = TRUE;
                }
                if (coinAvailable) {
                    Hu3DModelPosGet(cell->model, &cellPosition);
                    horizontalDistance = fabs(cellPosition.x - position->x);
                    coinCandidates[candidateCount].cell = cell;
                    coinCandidates[candidateCount].distance = horizontalDistance;
                    coinCandidates[candidateCount].row = (s16)row;
                    coinCandidates[candidateCount].column = (s16)column;
                    candidateCount++;
                }
            }
        }
        if (candidateCount != 0) break;
    }
    for (candidateIndex = 0; candidateIndex < candidateCount; candidateIndex++) {
        for (otherIndex = candidateIndex + 1; otherIndex < candidateCount; otherIndex++) {
            coinAvailable =
                coinCandidates[candidateIndex].distance > coinCandidates[otherIndex].distance
                    ? TRUE
                    : FALSE;
            if (coinAvailable) {
                candidateSwap = coinCandidates[candidateIndex];
                coinCandidates[candidateIndex] = coinCandidates[otherIndex];
                coinCandidates[otherIndex] = candidateSwap;
            }
        }
    }
    if (candidateCount == 0) return FALSE;
    if (candidateCount == 1) {
        selectedIndex = 0;
    } else {
        if (flags & 1) {
            if (flags & 4) {
                if (selectionBlocked) selectedIndex = 0;
                else selectedIndex = frandmod(2);
            } else {
                selectedIndex = frandmod(candidateCount);
            }
        } else {
            if (flags & 4) selectedIndex = 0;
            else selectedIndex = frandmod(2);
        }
    }
    selectedCell = coinCandidates[selectedIndex].cell;
    if (flags & 3) {
        lbl_1_bss_2C->selection[0] = coinCandidates[selectedIndex].row;
        lbl_1_bss_2C->selection[1] = coinCandidates[selectedIndex].column;
    }
    Hu3DModelPosGet(selectedCell->model, target);
    if (flags & 2) {
        horizontalDistance = target->x - position->x;
        target->x += 100.0f * (!(horizontalDistance > 0.0f) ? -1 : 1);
        target->y += 200.0f;
    }
    return TRUE;
}

/* Counts coins in this row and the next two, within two columns, to rank CPU empty targets. */
s32 fn_1_1ABC(s32 team, s32 row, s32 column)
{
    s32 columnRadius, rowRadius;
    M642GridCell *cell;
    s16 columnMin, columnMax, rowMin, rowMax, coinCount, scanRow, scanColumn;
    columnRadius = 2;
    rowRadius = 2;
    rowMin = row;
    if (rowMin < 0) rowMin = 0;
    rowMax = row + rowRadius;
    if (rowMax >= 8) rowMax = 8;
    columnMin = column - columnRadius;
    if (columnMin < 0) columnMin = 0;
    columnMax = column + columnRadius;
    if (columnMax >= 8) columnMax = 8;
    coinCount = 0;
    for (scanRow = rowMin; scanRow <= rowMax; scanRow++) {
        for (scanColumn = columnMin; scanColumn <= columnMax; scanColumn++) {
            cell = lbl_1_bss_18->grid[team][scanRow][scanColumn];
            if (cell && !(cell->flags & 4)) coinCount++;
        }
    }
    return coinCount;
}

/* Chooses a nearby empty grid position during CPU descent, preferring fewer nearby coins. */
BOOL fn_1_1BC8(s32 team, Vec *position, Vec *target)
{
    extern s32 fn_1_1ABC(s32 team, s32 row, s32 column);
    M642EmptyCandidate candidates[9];
    Vec delta;
    M642EmptyCandidate candidateSwap;
    s32 firstRow, candidateCount, row, column, candidateIndex, otherIndex, selectedIndex;
    BOOL eligible;
    M642GridCell *unusedCell;
    M642EmptyCandidate *selectedCandidate;

    firstRow = (5400.0f - position->y) / 200.0f;
    firstRow = abs(firstRow);
    if (firstRow >= 9) return FALSE;
    candidateCount = 0;
    unusedCell = NULL;
    for (row = firstRow; row <= firstRow; row++) {
        for (column = 0; column < 9; column++) {
            M642GridCell *cell = lbl_1_bss_18->grid[team][row][column];
            if (!cell) {
                eligible = TRUE;
            } else if (cell->flags & 4) {
                eligible = TRUE;
            }
            /* An occupied cell leaves eligibility unchanged from the preceding column. */
            if (eligible) {
                candidates[candidateCount].x = 50.0f + (-450.0f + 100.0f * column);
                candidates[candidateCount].y = 5400.0f - 200.0f * row - 100.0f;
                candidates[candidateCount].priority = fn_1_1ABC(team, row, column);
                delta.x = candidates[candidateCount].x - position->x;
                delta.y = candidates[candidateCount].y - position->y;
                candidates[candidateCount].distance = sqrtf(delta.x * delta.x + delta.y * delta.y);
                if (candidates[candidateCount].distance < 40.0f) candidateCount++;
            }
        }
        if (candidateCount != 0) break;
    }
    for (candidateIndex = 0; candidateIndex < candidateCount; candidateIndex++) {
        for (otherIndex = candidateIndex + 1; otherIndex < candidateCount; otherIndex++) {
            eligible = FALSE;
            if (candidates[candidateIndex].priority > candidates[otherIndex].priority)
                eligible = TRUE;
            if (eligible) {
                candidateSwap = candidates[candidateIndex];
                candidates[candidateIndex] = candidates[otherIndex];
                candidates[otherIndex] = candidateSwap;
            }
        }
    }
    if (candidateCount == 0) return FALSE;
    if (candidateCount == 1) {
        selectedCandidate = candidates;
    } else {
        selectedIndex = frandmod(2);
        selectedCandidate = &candidates[selectedIndex];
    }
    target->x = selectedCandidate->x;
    target->y = selectedCandidate->y;
    target->z = 0.0f;
    return TRUE;
}

s16 _CharFXPlay(s16 charNo, s16 seNo, u8 voiceFlag);
extern void fn_1_29BC(M642PlayerWork *work, s32 motion, float duration, u32 attr);
extern s32 fn_1_8F9C(Vec *position, s32 camera, float min, float max, s32 sound);
extern s32 fn_1_9008(Vec *position, float min, float max, s32 sound);

/* Scene motion hook: processes the next script group when its timing flag is set, synchronizing
 * character motions, sounds, dust and rumble for the teams. */
void fn_1_1F98(HU3D_MODELID model, HU3D_MOTIONID motion, BOOL lagF)
{
    s32 scriptMotions[6];
    Vec position;
    M642PlayerWork *firstPlayers[2];
    M642PlayerWork *secondPlayers[2];
    M642PlayerWork *work;
    s32 sound;
    s32 camera;
    f32 panMin;
    f32 panMax;
    s32 teamIndex;
    s32 motionIndex;
    BOOL processActions;
    u32 scriptAction;

    firstPlayers[0] = lbl_1_bss_18->players[0][0];
    firstPlayers[1] = lbl_1_bss_18->players[1][0];
    secondPlayers[0] = lbl_1_bss_18->players[0][1];
    secondPlayers[1] = lbl_1_bss_18->players[1][1];
    if (lagF) {
        if (lbl_1_bss_18->motion != motion) {
            lbl_1_bss_18->motion = motion;
            lbl_1_bss_18->step = 0;
            scriptMotions[0] = lbl_1_bss_2C->scene_objects[3]->mtnId[1];
            scriptMotions[1] = lbl_1_bss_2C->scene_objects[3]->mtnId[3];
            scriptMotions[2] = lbl_1_bss_2C->scene_objects[3]->mtnId[4];
            scriptMotions[3] = lbl_1_bss_2C->scene_objects[3]->mtnId[5];
            scriptMotions[4] = lbl_1_bss_2C->scene_objects[3]->mtnId[6];
            scriptMotions[5] = lbl_1_bss_2C->scene_objects[3]->mtnId[7];
            lbl_1_bss_18->row = -1;
            for (motionIndex = 0; motionIndex < 6; motionIndex++) {
                if (motion == scriptMotions[motionIndex]) {
                    lbl_1_bss_18->row = motionIndex;
                    break;
                }
            }
        }
        processActions = TRUE;
        while (processActions) {
            scriptAction = lbl_1_data_12C[lbl_1_bss_18->row][lbl_1_bss_18->step];
            switch (scriptAction) {
            case 11:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(firstPlayers[teamIndex], 3, 5.0f, 0);
                    if (teamIndex == 0) {
                        work = firstPlayers[teamIndex];
                        sound = CHARSEID(16);
                        _CharFXPlay(work->player->charNo, sound, 0);
                    }
                }
                break;
            case 12:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(firstPlayers[teamIndex], 0, 5.0f, 0);
                    MgActorPosGet(firstPlayers[teamIndex]->player->actor, &position);
                    CharModelLandDustCreate(firstPlayers[teamIndex]->charNo, &position);
                    if (teamIndex == 0) {
                        work = firstPlayers[teamIndex];
                        camera = work->player->camBit;
                        sound =
                            camera == 1 ? MSM_SE_CASHAPULT_LAND_LEFT : MSM_SE_CASHAPULT_LAND_RIGHT;
                        panMin = 0.0f;
                        panMax = 640.0f;
                        fn_1_9008(&position, panMin, panMax, sound);
                    }
                }
                break;
            case 1:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(firstPlayers[teamIndex], 3, 5.0f, 0);
                    work = firstPlayers[teamIndex];
                    camera = work->player->camBit;
                    sound = camera == 1 ? MSM_SE_CASHAPULT_JUMP_LEFT : MSM_SE_CASHAPULT_JUMP_RIGHT;
                    panMin = 0.0f;
                    panMax = 640.0f;
                    MgActorPosGet(work->player->actor, &position);
                    fn_1_8F9C(&position, camera, panMin, panMax, sound);
                }
                break;
            case 2:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(firstPlayers[teamIndex], 4, 5.0f, 0);
                }
                break;
            case 3:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(firstPlayers[teamIndex], 0, 5.0f, 0);
                    MgActorPosGet(firstPlayers[teamIndex]->player->actor, &position);
                    CharModelLandDustCreate(firstPlayers[teamIndex]->charNo, &position);
                    work = firstPlayers[teamIndex];
                    camera = work->player->camBit;
                    sound = camera == 1 ? MSM_SE_CASHAPULT_LAND_LEFT : MSM_SE_CASHAPULT_LAND_RIGHT;
                    panMin = 0.0f;
                    panMax = 640.0f;
                    fn_1_8F9C(&position, camera, panMin, panMax, sound);
                }
                break;
            case 4:
                Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[4]->mdlId[0],
                                   lbl_1_bss_2C->scene_objects[4]->mtnId[1], 0.0f, 0.0f, 0);
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    if (teamIndex == 0) {
                        work = firstPlayers[teamIndex];
                        camera = work->player->camBit;
                        sound = camera == 1 ? MSM_SE_CASHAPULT_LAUNCHER_LEFT
                                            : MSM_SE_CASHAPULT_LAUNCHER_RIGHT;
                        panMin = 0.0f;
                        panMax = 640.0f;
                        MgActorPosGet(firstPlayers[teamIndex]->player->actor, &position);
                        fn_1_9008(&position, panMin, panMax, sound);
                    }
                }
                break;
            case 13:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(secondPlayers[teamIndex], 3, 5.0f, 0);
                    work = secondPlayers[teamIndex];
                    sound = CHARSEID(16);
                    _CharFXPlay(work->player->charNo, sound, 0);
                }
                break;
            case 5:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(secondPlayers[teamIndex], 3, 5.0f, 0);
                    work = secondPlayers[teamIndex];
                    camera = work->player->camBit;
                    sound = camera == 1 ? MSM_SE_CASHAPULT_JUMP_LEFT : MSM_SE_CASHAPULT_JUMP_RIGHT;
                    panMin = 0.0f;
                    panMax = 640.0f;
                    MgActorPosGet(work->player->actor, &position);
                    fn_1_8F9C(&position, camera, panMin, panMax, sound);
                }
                break;
            case 6:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(secondPlayers[teamIndex], 4, 5.0f, 0);
                }
                break;
            case 7:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(secondPlayers[teamIndex], 0, 5.0f, 0);
                    MgActorPosGet(secondPlayers[teamIndex]->player->actor, &position);
                    CharModelLandDustCreate(secondPlayers[teamIndex]->charNo, &position);
                    work = secondPlayers[teamIndex];
                    camera = work->player->camBit;
                    sound = camera == 1 ? MSM_SE_CASHAPULT_LAND_LEFT : MSM_SE_CASHAPULT_LAND_RIGHT;
                    panMin = 0.0f;
                    panMax = 640.0f;
                    fn_1_8F9C(&position, camera, panMin, panMax, sound);
                }
                break;
            case 14:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    fn_1_29BC(secondPlayers[teamIndex], 0, 5.0f, 0);
                    MgActorPosGet(secondPlayers[teamIndex]->player->actor, &position);
                    CharModelLandDustCreate(secondPlayers[teamIndex]->charNo, &position);
                    work = secondPlayers[teamIndex];
                    sound = CHARSEID(8);
                    _CharFXPlay(work->player->charNo, sound, 0);
                }
                break;
            case 8:
                Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[5]->mdlId[0],
                                   lbl_1_bss_2C->scene_objects[5]->mtnId[1], 0.0f, 0.0f, 0);
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    work = secondPlayers[teamIndex];
                    camera = work->player->camBit;
                    sound = camera == 1 ? MSM_SE_CASHAPULT_LAUNCHER_LEFT
                                        : MSM_SE_CASHAPULT_LAUNCHER_RIGHT;
                    panMin = 0.0f;
                    panMax = 640.0f;
                    /* This scriptAction does not refresh the position sampled by earlier
                     * actions. */
                    fn_1_8F9C(&position, camera, panMin, panMax, sound);
                }
                break;
            case 9:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    omVibrate(firstPlayers[teamIndex]->player->playerNo, 20, 4, 4);
                }
                break;
            case 10:
                for (teamIndex = 0; teamIndex < 2; teamIndex++) {
                    omVibrate(secondPlayers[teamIndex]->player->playerNo, 20, 4, 4);
                }
                break;
            case 0:
                processActions = FALSE;
                break;
            }
            lbl_1_bss_18->step++;
        }
    }
}

/* Changes a player's character motion only when it differs from the current motion. */
void fn_1_29BC(M642PlayerWork *work, s32 motion, float duration, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        CharMotionShiftSet(work->charNo, work->player->omObj->mtnId[motion],
                           0.0f, duration, attr);
    }
}

/* Reports the direction when a moving point crosses a vertical target during a jump update. */
f32 fn_1_2A38(const HuVecF *previousPosition, const HuVecF *currentPosition, f32 targetY)
{
    f32 difference = currentPosition->y - previousPosition->y;
    if (difference > 0.0f) {
        if (previousPosition->y < targetY && currentPosition->y >= targetY) {
            return 1.0f;
        }
    } else if (difference < 0.0f && previousPosition->y > targetY &&
               currentPosition->y <= targetY) {
        return -1.0f;
    }
    return 0.0f;
}

/* Sets the player's current and next action when round callbacks start or reset a phase. */
void fn_1_2AEC(M642PlayerWork *work, s32 nextState)
{
    work->state = work->nextState = nextState;
}

/* Promotes a player queued action to the current action between round phases. */
void fn_1_2AFC(u8 *state)
{
    ((M642PlayerWork *)state)->state =
        ((M642PlayerWork *)state)->nextState;
}

typedef struct M642FlagState_s {
    u16 flags; /* Mode bits queried by the state test and set by the camera delay. */
    s16 state; /* Camera-delay counter; zero suppresses flag-query results. */
} M642FlagState;

/* Returns requested mode bits only while the camera-delay counter is nonzero. */
s32 fn_1_2B08(M642FlagState *state, s32 mask)
{
    if (state->state == 0) {
        return 0;
    }
    return state->flags & mask;
}

typedef struct M642ParticleConfigView_s {
    u32 flags; /* Particle behavior bits, including alternating-size animation. */
    GXColor endColor, startColor; /* Target and initial RGB tints; emission uses the target tint. */
    Vec velocity, decay; /* Initial world velocity and per-frame velocity multipliers. */
    f32 gravity, scale, scaleDelta, alphaDelta, colorTime; /* Vertical acceleration, size setting,
        * size change per frame, alpha change per frame and tint interpolation fraction. */
} M642ParticleConfigView;

M642ParticleConfigView lbl_1_data_450 = {
    0,
    {255, 255, 255, 255},
    {255, 255, 255, 255},
    {0.0f, 2.0f, 1.0f},
    {0.800000011920929f, 0.800000011920929f, 0.800000011920929f},
    0.20000000298023224f, 10.0f, 8.0f, -8.0f, 0.019999999552965164f
};
f32 lbl_1_data_488[2] = {2700.0f, 1500.0f};
f32 lbl_1_data_490[2] = {2750.0f, 1450.0f};
f32 lbl_1_data_498[2] = {1.0f, -1.0f};

extern s32 fn_1_8A50(Vec *source, Vec *destination);
extern s16 fn_1_7D1C(M642ParticleOwner *owner, s16 camera, f32 x, f32 y, f32 z,
                     f32 scale, M642ParticleConfigView *config);

/* Grid height-crossing updates emit a twelve-particle burst around the supplied world point. */
void fn_1_2B28(s32 team, f32 size, Vec *origin, f32 heightScale)
{
    Vec position;
    f32 speed;
    Vec particleDirection;
    Vec randomDirection;
    s32 camera = ! team ? 1 : 2;
    f32 randomizedSpeed;
    s32 particleCount = 12;
    f32 particleScale;
    f32 spread;
    f32 particleRange;
    f32 drag;
    f32 zero;
    f32 speedRange;
    f32 stepAngle;
    f32 randomSpread;
    f32 elevation;
    f32 velocityBias;
    f32 particleRandom;
    f32 speedRandom;
    f32 gravityScale;
    f32 sine;
    f32 cosine;
    s16 index;
    spread = 70.0f;
    randomSpread = 70.0f;
    drag = 0.800000011920929f;
    zero = 0.0f;
    velocityBias = 20.0f;
    gravityScale = 0.009999999776482582f;
    stepAngle = 360.0f / particleCount;
    speedRange = 10.0f;
    speedRandom = 10.0f;
    elevation = 60.0f;
    particleRange = 30.0f;
    particleRandom = 10.0f;
    for (index = 0; index < particleCount; index++)
    {
        sine = (f32) sin(3.141592653589793 * (stepAngle * index) / 180.0);
        cosine = (f32) cos(3.141592653589793 * (stepAngle * index) / 180.0);
        speed = size * (speedRange + (f32)(u32) frandmod((u32) speedRandom));
        randomDirection.x = speed;
        randomDirection.y =
            (f32) (((f64) speed / sin(3.141592653589793 * (90.0f - elevation) / 180.0)) *
                   sin(3.141592653589793 * elevation / 180.0));
        randomDirection.z = 0.0f;
        fn_1_8A50(& randomDirection, & randomDirection);
        particleDirection.x = sine * randomDirection.x;
        particleDirection.y = randomDirection.y * heightScale;
        particleDirection.z = cosine * randomDirection.x;
        fn_1_8A50(& particleDirection, & particleDirection);
        position.x = origin->x + sine * speed;
        position.y = origin->y;
        position.z = origin->z + cosine * speed;
        /* This additional speed draw advances the random stream even though its result is
         * unused. */
        randomizedSpeed = size * (spread + (f32)(u32) frandmod((u32) randomSpread));
        lbl_1_data_450.velocity.x = particleDirection.x * spread;
        lbl_1_data_450.velocity.y = particleDirection.y * spread;
        lbl_1_data_450.velocity.z = particleDirection.z * spread;
        lbl_1_data_450.decay.x = drag;
        lbl_1_data_450.decay.y = drag;
        lbl_1_data_450.decay.z = drag;
        lbl_1_data_450.gravity = (zero + velocityBias) * gravityScale;
        particleScale = size * (particleRange + (f32)(u32) frandmod((u32) particleRandom));
        fn_1_7D1C(team + ((M642SceneWork *) lbl_1_bss_2C)->owners, camera, position.x, position.y,
                  position.z, particleScale, &lbl_1_data_450);
    }
}

typedef struct M667Pool M667Pool;

/* Reserves and resets an inactive pooled effect record of the requested type. */
M642EffectRecord *fn_1_302C(s32 type)
{
    M642EffectRecord *effect;
    effect = fn_1_8794(lbl_1_bss_C->pool);
    while (effect) {
        if (!(effect->flags & 1) && effect->type == type) {
            effect->flags |= 1;
            effect->frame = 0;
            return effect;
        }
        effect = fn_1_87D0(lbl_1_bss_C->pool, effect);
    }
    return NULL;
}

/* Clears the effect's active bit when the effect animation finishes. */
void fn_1_30D8(u16 *value)
{
    *value = *value & M642_EFFECT_INACTIVE_MASK;
}

extern M642EffectRecord *fn_1_302C(s32 type);

/* Starts a type-zero effect at a player's position on that team's camera. */
M642EffectRecord *fn_1_30EC(s32 player, Vec *position)
{
    M642EffectRecord *effect;
    effect = fn_1_302C(0);
    if (effect) {
        effect->camera = player == 0 ? 1 : 2;
        effect->phase = 0;
        effect->frame = 0;
        effect->position = *position;
        return effect;
    }
    return NULL;
}

void fn_1_30D8(u16 *value);

/* The effect update calls this after its track ends to release the record and hide its model. */
void fn_1_3190(M642EffectRecord *data)
{
    fn_1_30D8(&data->flags);
    Hu3DModelAttrSet(data->model, HU3D_ATTR_DISPOFF);
}

void fn_1_140(OMOBJ *, OMOBJ_FUNC);
void fn_1_33D4(OMOBJ *);
void fn_1_42D0(OMOBJ *);
void fn_1_47C(void *);                           /* static */
void fn_1_4C60(OMOBJ *);
void fn_1_4E5C(OMOBJ *);
void fn_1_4EC4(OMOBJ *);
void fn_1_51D4(OMOBJ *);
void fn_1_7558(OMOBJ *);
OMOBJ *fn_1_84D8();                                    /* static */
void *fn_1_8568();                                    /* static */
void *fn_1_859C();                                    /* static */
s32 fn_1_91EC(s32);                         /* static */
s32 fn_1_9250(s32);                         /* static */
s32 fn_1_92B4();                                /* static */
void *fn_1_A0(s32, u32, void (*)(OMOBJ *));

/* Runs as the scene manager's first callback to set up shared state and create board, round, grid
 * and player objects. */
void fn_1_31CC(OMOBJ *object) {
    s32 playerIndex;
    M642SceneWork *sceneWork;

    sceneWork = object->data;
    fn_1_47C(sceneWork);
    fn_1_91EC(1);
    sceneWork->scene_objects[0] = fn_1_84D8();
    fn_1_91EC(2);
    sceneWork->scene_objects[1] = fn_1_84D8();
    fn_1_91EC(3);
    sceneWork->scene_objects[2] = fn_1_84D8();
    fn_1_91EC(4);
    sceneWork->scene_objects[3] = fn_1_84D8();
    fn_1_91EC(5);
    sceneWork->scene_objects[4] = fn_1_84D8();
    fn_1_91EC(6);
    sceneWork->scene_objects[5] = fn_1_84D8();
    fn_1_91EC(7);
    sceneWork->scene_objects[6] = fn_1_84D8();
    fn_1_91EC(8);
    sceneWork->scene_objects[7] = fn_1_84D8();
    fn_1_9250(0);
    sceneWork->owners = fn_1_8568();
    fn_1_92B4();
    sceneWork->spriteAssets = fn_1_859C();
    frandom(OSGetTick());
    lbl_1_bss_14 = fn_1_A0(10, 72, fn_1_42D0);
    lbl_1_bss_10 = fn_1_A0(10, 8, fn_1_4C60);
    lbl_1_bss_18 = fn_1_A0(10, 7916, fn_1_4EC4);
    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_1C[playerIndex] = fn_1_A0(20, 144, fn_1_51D4);
        lbl_1_bss_1C[playerIndex]->playerNo = playerIndex;
        playerIndex += 1;
    }
    lbl_1_bss_C = fn_1_A0(10, 4120, fn_1_7558);
    lbl_1_bss_8 = fn_1_A0(10, 8, fn_1_4E5C);
    sceneWork->sound = -1;
    sceneWork->soundState = 0;
    sceneWork->soundTimer = 0;
    fn_1_140(object, fn_1_33D4);
}

extern s32 fn_1_9120(s32 sound);
extern void fn_1_1F98(HU3D_MODELID model, s16 motion, BOOL enabled);
extern void fn_1_2AEC(M642PlayerWork *work, s32 state);
extern void fn_1_105C(void);
extern void fn_1_37B4(OMOBJ *object);
#define INTRO lbl_1_bss_2C
/* Intro object callback: starts the scene, measures team view bounds, then hands off to play. */
void fn_1_33D4(OMOBJ *object)
{
    M642SceneWork *work = object->data;
    s32 playerIndex;
    Vec screenPosition, worldPosition;
    if (MgSeqModeGet() == 2) {
        work->soundTimer++;
        switch (INTRO->soundState) {
        case 0:
            INTRO->sound = fn_1_9120(MSM_SE_CASHAPULT_INTRO);
            work->soundState = 1;
            break;
        case 1:
            if (work->soundTimer >= 210) {
                HuAudFXFadeOut(work->sound, 500);
                work->soundState = 2;
                INTRO->sound = -1;
            }
            break;
        }
    }
    switch (INTRO->phase) {
    case 0:
        if (MgSeqModeGet() == 2) {
            Hu3DMotionTimingHookSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], fn_1_1F98);
            Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0],
                               lbl_1_bss_2C->scene_objects[3]->mtnId[1], 0.0f, 0.0f, 0);
            Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0],
                               lbl_1_bss_2C->scene_objects[1]->mtnId[1], 0.0f, 0.0f, 0);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_1_2AEC(lbl_1_bss_1C[playerIndex], 1);
            }
            INTRO->phase++;
        }
        break;
    case 1:
        if (Hu3DMotionEndCheck(lbl_1_bss_2C->scene_objects[3]->mdlId[0])) {
            fn_1_105C();
            INTRO->phase++;
        }
        break;
    case 2:
        if (MgSeqModeGet() == 5) {
            screenPosition.x = 20.0f;
            screenPosition.y = 0.0f;
            screenPosition.z = 1000.0f;
            Hu3D2Dto3D(&screenPosition, 1, &worldPosition);
            lbl_1_bss_18->bounds[0].min = worldPosition.x;
            screenPosition.x = 540.0f;
            screenPosition.y = 0.0f;
            screenPosition.z = 1000.0f;
            Hu3D2Dto3D(&screenPosition, 1, &worldPosition);
            lbl_1_bss_18->bounds[0].max = worldPosition.x;
            screenPosition.x = 40.0f;
            screenPosition.y = 0.0f;
            screenPosition.z = 1000.0f;
            Hu3D2Dto3D(&screenPosition, 2, &worldPosition);
            lbl_1_bss_18->bounds[1].min = worldPosition.x;
            screenPosition.x = 560.0f;
            screenPosition.y = 0.0f;
            screenPosition.z = 1000.0f;
            Hu3D2Dto3D(&screenPosition, 2, &worldPosition);
            lbl_1_bss_18->bounds[1].max = worldPosition.x;
            work->timer = 0;
            fn_1_140(object, fn_1_37B4);
            return;
        }
        break;
    }
}

extern BOOL fn_1_C14(M642PlayerWork *work);
extern void fn_1_3A0C(OMOBJ *object);
/* Waits for the intro and player motions, then begins the first active turn. */
void fn_1_37B4(OMOBJ *object)
{
    s16 *timer = object->data;
    s32 playerIndex;
    if (*timer == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0],
                           lbl_1_bss_2C->scene_objects[3]->mtnId[3], 0.0f, 0.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0],
                           lbl_1_bss_2C->scene_objects[1]->mtnId[3], 0.0f, 0.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_2C->scene_objects[2]->mdlId[0],
                           lbl_1_bss_2C->scene_objects[2]->mtnId[1], 0.0f, 0.0f, 0);
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            fn_1_2AEC(lbl_1_bss_1C[playerIndex], 1);
        }
    }
    (*timer)++;
    if (Hu3DMotionEndCheck(lbl_1_bss_2C->scene_objects[3]->mdlId[0])) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (lbl_1_bss_1C[playerIndex]->state != 0) {
                return;
            }
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (fn_1_C14(lbl_1_bss_1C[playerIndex])) {
                fn_1_2AEC(lbl_1_bss_1C[playerIndex], 2);
            } else {
                fn_1_2AEC(lbl_1_bss_1C[playerIndex], 0);
            }
        }
        *timer = 0;
        fn_1_140(object, fn_1_3A0C);
    }
}

extern void fn_1_2AFC(u8 *state);
extern void fn_1_3AC4(OMOBJ *object);

/* Waits for all player callbacks to settle before selecting the next round phase. */
void fn_1_3A0C(OMOBJ *object)
{
    s16 *timer = object->data;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        fn_1_2AFC((u8 *)lbl_1_bss_1C[playerIndex]);
    }
    *timer = 0;
    fn_1_140(object, fn_1_3AC4);
}

extern void fn_1_3CF8(OMOBJ *object);

/* Sets the round's scene motions and pauses them before the turn phase begins. */
void fn_1_3AC4(OMOBJ *object)
{
    M642SceneWork *work = object->data;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    if (work->round < 3) {
        Hu3DMotionSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0],
            lbl_1_bss_2C->scene_objects[3]->mtnId[work->round + 4]);
        Hu3DMotionSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0],
            lbl_1_bss_2C->scene_objects[1]->mtnId[work->round + 4]);
        Hu3DMotionSet(lbl_1_bss_2C->scene_objects[2]->mdlId[0],
            lbl_1_bss_2C->scene_objects[2]->mtnId[work->round + 2]);
    } else {
        Hu3DMotionSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0],
            lbl_1_bss_2C->scene_objects[3]->mtnId[7]);
    }
    Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[2]->mdlId[0], 0.0f);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        fn_1_2AFC((u8 *)lbl_1_bss_1C[playerIndex]);
    }
    work->timer = 0;
    fn_1_140(object, fn_1_3CF8);
}

extern void fn_1_2AEC(M642PlayerWork *state, s32 next);
extern void fn_1_3ED8(OMOBJ *object);

/* Restarts round animations, clears the cell selection, and sends players to their turns. */
void fn_1_3CF8(OMOBJ *object)
{
    M642SceneWork *work = object->data;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    Hu3DMotionTimeSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], 0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], 1.0f);
    if (work->round < 3) {
        Hu3DMotionTimeSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0], 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[1]->mdlId[0], 1.0f);
        Hu3DMotionTimeSet(lbl_1_bss_2C->scene_objects[2]->mdlId[0], 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_2C->scene_objects[2]->mdlId[0], 1.0f);
    }
    lbl_1_bss_2C->selection[0] = -1;
    lbl_1_bss_2C->selection[1] = -1;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        fn_1_2AEC(lbl_1_bss_1C[playerIndex], 5);
    }
    work->timer = 0;
    fn_1_140(object, fn_1_3ED8);
}

extern void fn_1_D2C(void);
extern BOOL fn_1_C14(M642PlayerWork *player);
extern void fn_1_403C(OMOBJ *object);

/* Advances the round after player actions finish, or starts end-of-game handling. */
void fn_1_3ED8(OMOBJ *object)
{
    M642SceneWork *work = object->data;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    work->timer = 0;
    work->round++;
    fn_1_D2C();
    if (work->round < 4) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (fn_1_C14(lbl_1_bss_1C[playerIndex])) {
                fn_1_2AEC(lbl_1_bss_1C[playerIndex], 2);
            } else {
                fn_1_2AEC(lbl_1_bss_1C[playerIndex], 0);
            }
        }
        fn_1_140(object, fn_1_3A0C);
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            fn_1_2AEC(lbl_1_bss_1C[playerIndex], 6);
        }
        work->timer = 0;
        fn_1_140(object, fn_1_403C);
    }
}

extern void fn_1_40DC(OMOBJ *object);

/* Marks the game scene ready for results after every player's final motion finishes. */
void fn_1_403C(OMOBJ *object)
{
    s16 *timer = object->data;
    s32 playerIndex;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    lbl_1_bss_2C->state = M642_SCENE_PLAY_DONE;
    *timer = 0;
    fn_1_140(object, fn_1_40DC);
}

extern void fn_1_108C(void);
extern void fn_1_4180(OMOBJ *object);

/* Waits for the results phase after every player finishes, then starts the final callbacks. */
void fn_1_40DC(OMOBJ *object)
{
    s16 *timer = object->data;
    s32 i;
    (*timer)++;
    if (MgSeqModeGet() == 7) {
        fn_1_108C();
        for (i = 0; i < 4; i++) {
            fn_1_2AEC(lbl_1_bss_1C[i], 7);
        }
        *timer = 0;
        fn_1_140(object, fn_1_4180);
        return;
    }
}

extern s32 fn_1_A60(s32 team);
extern void fn_1_7878(s32 playerNo, s16 mgCoin);

/* Results callback: starts the jingle during a player's last motion, then awards each team's coins
 * once all players stop. */
void fn_1_4180(OMOBJ *object)
{
    void *unusedSceneWork = object->data;
    s32 playerIndex, teamCoins, team;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_1C[playerIndex]->state == 10 && lbl_1_bss_2C->sound == -1) {
            lbl_1_bss_2C->sound = HuAudJinglePlay(MSM_STREAM_MGMUS_17);
        }
        if (lbl_1_bss_1C[playerIndex]->state != 0) {
            return;
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        team = lbl_1_bss_1C[playerIndex]->team;
        teamCoins = fn_1_A60(team);
        fn_1_7878(lbl_1_bss_1C[playerIndex]->playerNo, teamCoins);
    }
    lbl_1_bss_2C->state = M642_SCENE_RESULTS_DONE;
    fn_1_140(object, NULL);
}

void fn_1_140(OMOBJ *, OMOBJ_FUNC);            /* static */
s32 fn_1_BE8();                                 /* static */

void fn_1_4BBC(OMOBJ *);

/* Configures the Cashapult cameras when the round-control object is created. */
void fn_1_42D0(OMOBJ *object) {
    s16 camera;
    M642RoundControl *cameraWork;
    cameraWork = object->data;
    camera = 3;
    Hu3DCameraCreate(camera);
    Hu3DCameraPerspectiveSet(camera, 45.0f, 20.0f,
        30000.0f, 1.2000000476837158f);
    Hu3DCameraViewportSet(camera, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f, 1.0f);
    cameraWork->teams[0].target.x = lbl_1_data_32C.x;
    cameraWork->teams[0].target.y = lbl_1_data_32C.y;
    cameraWork->teams[0].target.z = lbl_1_data_32C.z;
    cameraWork->teams[0].position.x = cameraWork->teams[0].target.x + lbl_1_data_344.x;
    cameraWork->teams[0].position.y = cameraWork->teams[0].target.y + lbl_1_data_344.y;
    cameraWork->teams[0].position.z = cameraWork->teams[0].target.z + lbl_1_data_344.z;
    Hu3DCameraPosSetV(1, &cameraWork->teams[0].position,
        &lbl_1_data_368, &cameraWork->teams[0].target);
    Hu3DCameraScissorSet(2, 0U, 0U, 0U, 0U);
    cameraWork->timer = 0;
    fn_1_140(object, fn_1_4BBC);
}

/* Camera mode-one callback: waits until its counter exceeds 30, then opens the split view. */
void fn_1_4460(OMOBJ *object)
{
    M642FlagState *state = object->data;
    state->state += 1;
    if (state->state > 30) {
        state->state = 0;
        state->flags = 2;
    }
}

void fn_1_89F8(Point3d *start, Point3d *end, f32 t, Point3d *out);

/* Camera transition callback: interpolates both team views across 60 frames, then waits for
 * play. */
void fn_1_44A0(OMOBJ *object)
{
    M642RoundControl *work = object->data;
    Vec offset;
    f32 progress;
    f32 firstScissorWidth;
    f32 secondViewportWidth;
    f32 firstLeft;
    f32 secondLeft;

    progress = (f32)work->timer / 60.0f;
    firstScissorWidth = 640.0f + -320.0f * progress;
    secondViewportWidth = 640.0f * progress;
    firstLeft = (640.0f + -304.0f * progress) / 2 - 320.0f;
    secondLeft = 640.0f - 3.0f * secondViewportWidth / 4 - 8.0f * progress;
    fn_1_89F8(&lbl_1_data_344, &lbl_1_data_350, progress, &offset);
    work->teams[0].target.x = lbl_1_data_32C.x;
    work->teams[0].target.y = lbl_1_data_32C.y;
    work->teams[0].target.z = lbl_1_data_32C.z;
    work->teams[0].position.x = work->teams[0].target.x + offset.x;
    work->teams[0].position.y = work->teams[0].target.y + offset.y;
    work->teams[0].position.z = work->teams[0].target.z + offset.z;
    Hu3DCameraPosSetV(1, &work->teams[0].position, &lbl_1_data_368, &work->teams[0].target);
    work->teams[1] = work->teams[0];
    Hu3DCameraPosSetV(2, &work->teams[1].position, &lbl_1_data_368, &work->teams[1].target);
    Hu3DCameraViewportSet(1, firstLeft, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
    if (firstScissorWidth > 2.0f) {
        Hu3DCameraScissorSet(1, 0, 0, (u32)firstScissorWidth - 2, 480);
    } else {
        Hu3DCameraScissorSet(1, 0, 0, 0, 480);
    }
    if (work->timer >= 1) {
        Hu3DCameraViewportSet(2, secondLeft, 0.0f, secondViewportWidth, 480.0f,
                              0.0f, 1.0f);
        if (firstScissorWidth > 2.0f) {
            Hu3DCameraScissorSet(2, (u32)firstScissorWidth + 2, 0,
                                  638 - (u32)firstScissorWidth, 480);
        } else {
            Hu3DCameraScissorSet(2, 0, 0, 0, 480);
        }
    }
    work->timer++;
    if (work->timer > 60) {
        work->timer = 0;
        work->mode = 3;
    }
}

/* Saves both team camera views when the sequence reaches phase 5. */
void fn_1_486C(OMOBJ *object)
{
    M642RoundControl *work;
    Vec up;
    if (MgSeqModeGet() == 5) {
        work = object->data;
        Hu3DCameraPosGet(1, &work->teams[0].position, &up, &work->teams[0].target);
        Hu3DCameraPosGet(2, &work->teams[1].position, &up, &work->teams[1].target);
        work->timer = 0;
        work->mode = 4;
    }
}

/* Tracks each team camera's height from its active player, or returns it to the default height. */
void fn_1_48E4(OMOBJ *object)
{
    M642RoundControl *work = object->data;
    Vec position, up, target, playerPosition;
    s32 teamIndex, camera;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        camera = teamIndex == 0 ? 1 : 2;
        Hu3DCameraPosGet(camera, &position, &up, &target);
        if (work->teams[teamIndex].active != 0) {
            MgActorPosGet(work->teams[teamIndex].active->player->actor, &playerPosition);
            if (playerPosition.y < 4900.0f) {
                target.y = playerPosition.y;
            } else {
                target.y = 4900.0f;
            }
            position.y = target.y + lbl_1_data_344.y;
            Hu3DCameraPosSetV(camera, &position, &up, &target);
        } else {
            target.y = 700.0f;
            position.y = target.y + lbl_1_data_344.y;
            Hu3DCameraPosSetV(camera, &position, &up, &target);
        }
    }
}

void fn_1_89F8(Point3d *start, Point3d *end, f32 t, Point3d *out);

/* Moves both team cameras to their next positions over 60 updates, then advances the camera
 * state. */
void fn_1_4A28(OMOBJ *object)
{
    M642RoundControl *work = object->data;
    Vec targetPoint;
    Vec cameraOffset;
    f32 progress;

    work->timer++;
    progress = work->timer / 60.0f;
    if (progress <= 1.0f) {
        fn_1_89F8(&lbl_1_data_32C, &lbl_1_data_338, progress, &targetPoint);
        fn_1_89F8(&lbl_1_data_350, &lbl_1_data_35C, progress, &cameraOffset);
        work->teams[0].target.x = targetPoint.x;
        work->teams[0].target.y = targetPoint.y;
        work->teams[0].target.z = targetPoint.z;
        work->teams[0].position.x = work->teams[0].target.x + cameraOffset.x;
        work->teams[0].position.y = work->teams[0].target.y + cameraOffset.y;
        work->teams[0].position.z = work->teams[0].target.z + cameraOffset.z;
        Hu3DCameraPosSetV(1, &work->teams[0].position, &lbl_1_data_368, &work->teams[0].target);
        work->teams[1] = work->teams[0];
        Hu3DCameraPosSetV(2, &work->teams[1].position, &lbl_1_data_368, &work->teams[1].target);
        return;
    }
    work->mode = 6;
}

extern void fn_1_4460(OMOBJ *obj);
extern void fn_1_44A0(OMOBJ *obj);
extern void fn_1_486C(OMOBJ *obj);
extern void fn_1_48E4(OMOBJ *obj);
extern void fn_1_4A28(OMOBJ *obj);

/* Dispatches the round camera callback for the current camera transition state. */
void fn_1_4BBC(OMOBJ *object)
{
    s16 *cameraState = object->data;

    switch (cameraState[0]) {
        case 1:
            fn_1_4460(object);
            break;
        case 2:
            fn_1_44A0(object);
            break;
        case 3:
            fn_1_486C(object);
            break;
        case 4:
            fn_1_48E4(object);
            break;
        case 5:
            fn_1_4A28(object);
            break;
        case 6:
            break;
    }
}

extern s16 fn_1_7890(void);
void fn_1_4E58(OMOBJ *object);

/* Sets the arena light and team shadows during scene setup, using the selected color palette. */
void fn_1_4C60(OMOBJ *object)
{
    u32 lightX, lightY, lightZ;
    u8 red, green, blue;
    s16 light;

    if (fn_1_7890() == 0) {
        lightX = 148;
        lightY = 224;
        lightZ = 1040;
        red = 229;
        green = 242;
        blue = 255;
    } else {
        lightX = 148;
        lightY = 224;
        lightZ = 1040;
        red = 114;
        green = 102;
        blue = 178;
    }

    /* The light API receives the low byte of each configured position coordinate. */
    light = Hu3DGLightCreate((u8)lightX, (u8)lightY, (u8)lightZ,
        -1.0f, -1.0f, -1.0f,
        red, green, blue);
    Hu3DGLightInfinitytSet(light);
    {
    HuVecF lightPosition = {-100.0f, 8000.0f, 0.0f};
    HuVecF shadowUp = {0.0f, 1.0f, 0.0f};
    HuVecF shadowTarget = {0.0f, 0.0f, 0.0f};
    Hu3DShadowMultiCreate(15.0f, 10.0f,
        10000.0f, 3);
    Hu3DShadowMultiColSet(16, 16, 16, 3);
    Hu3DShadowMultiPosSet(&lightPosition, &shadowUp, &shadowTarget, 3);
    Hu3DShadowMultiTPLvlSet(0.30000001192092896f, 3);
    }
    fn_1_140(object, fn_1_4E58);
}

/* Leaves the arena lighting unchanged on each update. */
void fn_1_4E58(OMOBJ *object)
{
}

extern void fn_1_6E4(M642ScoreConfig *data);
extern void fn_1_4EC0(OMOBJ *object);

/* Creates both teams' score displays and shows them when scoreboard setup begins. */
void fn_1_4E5C(OMOBJ *object)
{
    fn_1_6E4(lbl_1_data_30);
    fn_1_6E4(&lbl_1_data_30[1]);
    fn_1_914(0, 1);
    fn_1_914(1, 1);
    fn_1_140(object, fn_1_4EC0);
}

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

void fn_1_4EC0(OMOBJ *object)
{
}

/* Prepares the team grids and collision models, then installs their per-frame update callback. */
void fn_1_4EC4(OMOBJ *object)
{
    M642GridWork *work = object->data;
    COL_ATTRPARAM collisionParameters;
    s32 pattern;

    work->pools[0] = fn_1_8618((u8 *)&work->cellStorage[0], 3600, 48);
    work->pools[1] = fn_1_8618((u8 *)&work->cellStorage[1], 3600, 48);
    work->motion = -1;

    Hu3DModelLayerSet(lbl_1_bss_2C->scene_objects[4]->mdlId[0], 1);
    Hu3DModelLayerSet(lbl_1_bss_2C->scene_objects[5]->mdlId[0], 1);
    MgActorColMapInit(&lbl_1_bss_2C->scene_objects[0]->mdlId[1], 0, 40);
    Hu3DModelAttrSet(lbl_1_bss_2C->scene_objects[6]->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_2C->scene_objects[7]->mdlId[0], HU3D_ATTR_DISPOFF);

    pattern = frand() % 5;
    fn_1_11E4(0, pattern);
    fn_1_11E4(1, pattern);
    Hu3DModelCameraSet(lbl_1_bss_2C->scene_objects[0]->mdlId[2], 1U);
    Hu3DModelCameraSet(lbl_1_bss_2C->scene_objects[0]->mdlId[3], 2U);

    MgActorColAttrParamGet(&collisionParameters, 1U);
    collisionParameters.yDeviate = 0.0f;
    MgActorColAttrParamSet(&collisionParameters, 1U);
    fn_1_140(object, fn_1_5080);
}

extern float fn_1_2A38(const Vec *first, const Vec *second, float height);
extern void fn_1_2B28(s32 team, float scale, Vec *position, float yScale);
extern void *fn_1_CAC(s32 team);
extern void fn_1_E48(s32 team, void *player);
/* Checks player height crossings for effects and updates both teams' collectible-cell
 * collisions. */
void fn_1_5080(OMOBJ *object)
{
    s32 playerIndex, heightIndex;
    M642PlayerWork *playerWork;
    Vec playerPosition, burstPosition;
    float crossingDirection;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerWork = lbl_1_bss_1C[playerIndex];
        MgActorPosGet(playerWork->player->actor, &playerPosition);
        for (heightIndex = 0; heightIndex < 2; heightIndex++) {
            crossingDirection =
                fn_1_2A38(&playerWork->position, &playerPosition, lbl_1_data_490[heightIndex]);
            if (0.0f != crossingDirection) {
                burstPosition = playerPosition;
                burstPosition.y = lbl_1_data_488[heightIndex];
                fn_1_2B28(playerWork->team, 2.0f, &burstPosition, lbl_1_data_498[heightIndex]);
                break;
            }
        }
    }
    fn_1_E48(0, fn_1_CAC(0));
    fn_1_E48(1, fn_1_CAC(1));
}

extern s32 fn_1_594(M642PlayerWork *work);
extern void fn_1_A90(M642PlayerWork *work);
extern void fn_1_68F0(OMOBJ *object);
/* Creates a player's Cashapult character, assigns its team launcher and configures its controls. */
void fn_1_51D4(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    MGACTOR_PARAM actorParameters = {0};
    Vec lightPosition;
    s32 camera;

    work->timer = 0;
    work->charNo = GwPlayerConf[work->playerNo].charNo;
    work->team = fn_1_594(work);
    work->difficulty = GwPlayerConf[work->playerNo].comDif;
    work->state = 0;
    work->initialized = 0;
    camera = work->team == 0 ? 1 : 2;
    actorParameters.height = 150.0f;
    actorParameters.radius = 40.0f;
    actorParameters.param = work->playerNo;
    actorParameters.type = 0;
    actorParameters.attr = 14;
    actorParameters.correctHookParam = 0;
    actorParameters.narrowHook = NULL;
    actorParameters.correctHook = NULL;
    work->player = MgPlayerCreate(work->playerNo, &actorParameters, 2, camera, 1, lbl_1_data_D8);
    work->computer = GwPlayerConf[work->playerNo].type;
    work->difficulty = GwPlayerConf[work->playerNo].comDif;
    work->aiFrames = 0;
    MgPlayerComStkOn(work->player);
    MgPlayerModeFuncSet(work->player, lbl_1_data_3CC);
    MgActorColMaskSet(work->player->actor, work->team == 0 ? 1 : 2);
    Hu3DModelCameraSet(work->player->actor->mdlId, camera);
    Hu3DModelShadowSet(work->player->actor->mdlId);
    CharMotionVoiceOnSet(work->player->charNo, 3, FALSE);
    CharMotionVoiceOnSet(work->player->charNo, 4, FALSE);
    CharMotionVoiceOnSet(work->player->charNo, 117, FALSE);
    CharMotionUpdateSet(work->player->charNo, DATANUM(DATA_mariomot, 3), FALSE);
    CharMotionUpdateSet(work->player->charNo, DATANUM(DATA_mariomot, 4), FALSE);
    CharMotionUpdateSet(work->player->charNo, DATANUM(DATA_mariomot, 117), FALSE);
    CharEffectLayerSet(1);
    fn_1_A90(work);
    lightPosition = work->position;
    lightPosition.x += lbl_1_data_30C.x;
    lightPosition.y += lbl_1_data_30C.y;
    lightPosition.z += lbl_1_data_30C.z;
    Hu3DLLightCreateV(work->player->actor->mdlId, &lightPosition, &lbl_1_data_318,
                      &lbl_1_data_324[(s16) fn_1_7890()]);
    if (work->computer) {
        work->aiParams = &lbl_1_data_F4[work->difficulty];
    }
    work->sound = -1;
    work->timer = 0;
    fn_1_140(object, fn_1_68F0);
}

/* Follows the player's launcher animation and resets the player state when that motion ends. */
void fn_1_5564(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position = {0.0f, 0.0f, 0.0f};
    MGACTOR *actor = work->player->actor;

    if (!Hu3DMotionEndCheck(lbl_1_bss_2C->scene_objects[3]->mdlId[0])) {
        Hu3DModelObjPosGet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], lbl_1_data_124[work->slot],
                           &position);
        if (fn_1_C14(work)) {
            if (position.y < 700.0f) {
                work->flags |= 1;
            } else if (lbl_1_bss_14->teams[work->team].active == 0 &&
                       (work->flags & 1) && position.y >= 700.0f && work->velocity.y > 0.0f) {
                lbl_1_bss_14->teams[work->team].active = work;
            }
        }
        MgActorPosSet(actor, &position);
        work->velocity.y = position.y - work->position.y;
    } else {
        position = work->position;
        position.y += work->velocity.y;
        MgActorPosSet(actor, &position);
        work->aiFrames = 0;
        work->steeringFrames = 0;
        work->timer = 0;
        work->nextState = 0;
        work->state = 0;
        work->flags &= ~4;
    }
}

extern void fn_1_89F8(Point3d *start, Point3d *end, f32 t, Point3d *out);
extern M642EffectRecord *fn_1_30EC(s32 team, Vec *position);
/* Player launch callback: rises to height 2600 with sound and smoke, then queues free-flight
 * steering. */
void fn_1_577C(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position = work->position;
    Vec direction, smoke;
    s32 camera, sound;
    float panMin, panMax;

    if (position.y < 2600.0f) {
        position.y += work->velocity.y;
    }
    work->velocity.y += 0.5f;
    if (work->velocity.y > 56.0f) {
        work->velocity.y = 56.0f;
    }
    if (work->timer == 0 && work->sound == -1) {
        camera = work->player->camBit;
        sound = camera == 1 ? MSM_SE_CASHAPULT_FLIGHT_LEFT : MSM_SE_CASHAPULT_FLIGHT_RIGHT;
        panMin = 0.0f;
        panMax = 640.0f;
        work->sound = fn_1_8F9C(&position, camera, panMin, panMax, sound);
    }
    if (position.y >= 1500.0f && position.y <= 2300.0f && work->timer % 2 == 0) {
        direction = work->velocity;
        VECNormalize(&direction, &direction);
        smoke.x = position.x + 2.0f * (100.0f * direction.x);
        smoke.y = position.y + 2.0f * (100.0f * direction.y);
        smoke.z = position.z + 2.0f * (100.0f * direction.z);
        fn_1_30EC(work->team, &smoke);
    }
    work->timer++;
    MgActorPosSet(work->player->actor, &position);
    if (position.y < 2600.0f) {
        return;
    }
    if (work->sound != -1) {
        HuAudFXStop(work->sound);
        work->sound = -1;
    }
    work->timer = 0;
    work->nextState = 3;
    work->state = 0;
    work->aiState = 0;
    work->direction.x = 0.0f;
    work->direction.z = 1.0f;
    work->flags &= ~4;
    work->flags &= ~(1 << 12);
}

extern f32 fn_1_320(Vec *direction);
extern f32 fn_1_378(f32 start, f32 end, f32 t);
extern void fn_1_74B4(OMOBJ *object);

/* Player flight callback: applies horizontal steering and gravity, then queues the launcher return
 * at or below height 2400. */
void fn_1_5A84(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position;
    MGPLAYER *player = work->player;
    f32 heightDifference;
    f32 angleMagnitude;
    f32 directionSign;

    MgActorPosGet(work->player->actor, &position);
    if (work->timer == 0) {
        heightDifference = 5400.0f - position.y;
        work->gravity = work->velocity.y * work->velocity.y / heightDifference / 2;
        work->velocity.x = 0.0f;
        work->velocity.z = 0.0f;
        work->nextStep.x = 0.0f;
        work->nextStep.z = 0.0f;
    }
    work->direction.x = 0.0f;
    work->direction.z = 0.0f;
    if (work->computer != 0) {
        fn_1_74B4(object);
    } else {
        work->direction.x = (s8)HuPadStkX[(s8)player->padNo];
        work->direction.y = 0.0f;
        work->direction.z = -(s8)HuPadStkY[(s8)player->padNo];
    }
    angleMagnitude =
        sqrtf(work->direction.x * work->direction.x + work->direction.z * work->direction.z);
    if (angleMagnitude != 0.0f) {
        work->rotation = fn_1_378(work->rotation, fn_1_320(&work->direction), 0.20000000298023224f);
    }
    MgActorRotYSet(work->player->actor, work->rotation);
    directionSign = 0.0f;
    if (work->direction.x > 0.0f) {
        directionSign = 1.0f;
    } else if (work->direction.x < 0.0f) {
        directionSign = -1.0f;
    }
    work->velocity.x += 0.30000001192092896f * directionSign;
    if (work->velocity.x * directionSign > 0.0f &&
        fabs(work->velocity.x) > 7.0) {
        work->velocity.x = 7.0f * directionSign;
    }
    if (work->velocity.y > 0.0f && work->velocity.y <= work->gravity) {
        work->flags &= ~4;
    }
    position.x += work->velocity.x;
    position.y += work->velocity.y;
    work->velocity.y -= work->gravity;
    if (position.x < lbl_1_bss_18->bounds[work->team].min) {
        work->velocity.x = 0.0f;
        position.x = lbl_1_bss_18->bounds[work->team].min;
    } else if (position.x > lbl_1_bss_18->bounds[work->team].max) {
        work->velocity.x = 0.0f;
        position.x = lbl_1_bss_18->bounds[work->team].max;
    }
    MgActorPosSet(work->player->actor, &position);
    work->timer++;
    if (!(position.y > 2400.0f)) {
        player = work->player;
        player->actor->gravity = 0.0f;
        player->actor->velY = 0.0f;
        MgPlayerComStkOn(player);
        work->jumps++;
        work->aiState = 0;
        work->timer = 0;
        work->nextState = 4;
        work->state = 0;
        work->flags &= ~4;
    }
}

extern s32 fn_1_8F9C(Vec *position, s32 camera, f32 min, f32 max, s32 sound);
extern s16 fn_1_8E68(Vec *position, s32 camera, f32 min, f32 max);

/* Player return callback: interpolates to the launcher over 60 frames with travel sound and
 * smoke. */
void fn_1_5F5C(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position;
    Vec direction;
    f32 progress;
    f32 minPan, maxPan;
    s32 camera;
    s32 sound, effect;
    s16 pan;

    if (work->timer == 0) {
        work->startingPosition = work->position;
        Hu3DModelObjPosGet(lbl_1_bss_2C->scene_objects[3]->mdlId[0],
                           lbl_1_data_124[work->slot], &work->targetPosition);
    }
    progress = (f32)work->timer / 60.0f;
    fn_1_89F8(&work->startingPosition, &work->targetPosition, progress, &position);
    camera = work->player->camBit;
    sound = camera == 1 ? MSM_SE_CASHAPULT_FLIGHT_LEFT : MSM_SE_CASHAPULT_FLIGHT_RIGHT;
    minPan = 0.0f;
    maxPan = 640.0f;
    if (work->timer == 0) {
        if (work->sound == -1) {
            work->sound = fn_1_8F9C(&position, camera, minPan, maxPan, sound);
        }
    } else if (work->sound != -1) {
        pan = fn_1_8E68(&position, camera, minPan, maxPan);
        HuAudFXPanning(work->sound, pan);
    }
    MgActorPosSet(work->player->actor, &position);
    work->timer++;
    if (position.y >= 1650.0f && work->timer % 5 == 0) {
        PSVECSubtract(&work->startingPosition, &work->targetPosition, &direction);
        PSVECNormalize(&direction, &direction);
        position.x -= 30.0f * direction.x;
        position.y -= 30.0f * direction.y;
        position.z -= 30.0f * direction.z;
        effect = (s32)fn_1_30EC(work->team, &position);
    }
    if (!(progress < 1.0f)) {
        if (work->sound != -1) {
            HuAudFXStop(work->sound);
            work->sound = -1;
        }
        MgActorRotYSet(work->player->actor, work->baseRotation);
        work->timer = 0;
        work->nextState = 5;
        work->state = 0;
        work->flags &= ~4;
    }
}

/* Tracks a player's launcher motion and updates the team's active-player slot until it ends. */
void fn_1_6268(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position = {0.0f, 0.0f, 0.0f};
    MGACTOR *actor = work->player->actor;
    s32 team;

    if (!Hu3DMotionEndCheck(lbl_1_bss_2C->scene_objects[3]->mdlId[0])) {
        Hu3DModelObjPosGet(lbl_1_bss_2C->scene_objects[3]->mdlId[0], lbl_1_data_124[work->slot],
                           &position);
        team = work->team;
        if (lbl_1_bss_14->teams[team].active == work) {
            if (position.y <= 700.0f) {
                lbl_1_bss_14->teams[team].active = NULL;
            }
        } else if (position.y >= 700.0f) {
            lbl_1_bss_14->teams[team].active = work;
        }
        MgActorPosSet(work->player->actor, &position);
        work->velocity.y = position.y - work->position.y;
    } else {
        if (work->velocity.y > 0.0f) {
            position = work->position;
            position.y += work->velocity.y;
            MgActorPosSet(actor, &position);
        }
        work->timer = 0;
        work->state = 0;
        work->flags &= ~4;
    }
}

/* Clears the player's action and timer when its reset motion is idle. */
void fn_1_645C(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position = {0.0f, 0.0f, 0.0f};
    if (work->motion == 0) {
        work->timer = 0;
        work->state = 0;
    }
}

extern void fn_1_29BC(M642PlayerWork *work, s32 motion, float blend, u32 flags);
extern s32 fn_1_8A50(Vec *source, Vec *result);

/* Player results callback: prepares a route to the horizontal center and starts its looping
 * motion. */
void fn_1_64B0(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec initialPosition = {0.0f, 0.0f, 0.0f};
    Vec position, target;
    Vec initialDirection = {0.0f, 0.0f, 0.0f};
    position = work->position;
    target = position;
    target.x = 0.0f;
    fn_1_29BC(work, 2, 4.0f, HU3D_MOTATTR_LOOP);
    CharNpcDustVoiceOffSet(work->player->actor->mdlId, work->player->omObj->mtnId[2], 1);
    VECSubtract(&target, &position, &work->direction);
    fn_1_8A50(&work->direction, &work->direction);
    work->targetPosition = target;
    work->timer = 0;
    work->state = 8;
}

extern float fn_1_320(Vec *direction);
extern float fn_1_378(float start, float end, float t);

/* Turns and moves the player toward the prepared target, then starts the arrival motion nearby. */
void fn_1_65F0(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec position = {0.0f, 0.0f, 0.0f};
    Vec delta = {0.0f, 0.0f, 0.0f};
    float distanceSquared;

    work->rotation = fn_1_378(work->rotation, fn_1_320(&work->direction), 0.20000000298023224f);
    MgActorRotYSet(work->player->actor, work->rotation);
    MgActorPosGet(work->player->actor, &position);
    position.x += 5.599999904632568f * work->direction.x;
    position.z += 5.599999904632568f * work->direction.z;
    MgActorPosSet(work->player->actor, &position);
    PSVECSubtract(&work->targetPosition, &position, &delta);
    distanceSquared = delta.z * delta.z + (delta.x * delta.x + delta.y * delta.y);
    if (distanceSquared <= 8100.0f) {
        fn_1_29BC(work, 0, 4.0f, 0);
        work->timer = 0;
        work->state = 9;
    }
}

/* Player results callback: faces forward until the delay exceeds 40 frames, then starts the final
 * character motion. */
void fn_1_6788(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec initialPosition = {0.0f, 0.0f, 0.0f};
    Vec initialDirection = {0.0f, 0.0f, 0.0f};
    work->timer++;
    if (work->timer > 0) {
        work->direction.x = 0.0f;
        work->direction.z = 1.0f;
        work->rotation = fn_1_378(work->rotation, fn_1_320(&work->direction), 0.20000000298023224f);
        MgActorRotYSet(work->player->actor, work->rotation);
    }
    if (work->timer > 40) {
        fn_1_29BC(work, 5, 4.0f, 0);
        work->timer = 0;
        work->state = 10;
    }
}

/* Clears the player state when the final character motion ends during its update. */
void fn_1_68A0(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    if (CharMotionEndCheck(work->player->charNo)) {
        work->state = 0;
    }
}

void fn_1_5564(OMOBJ *object);
void fn_1_577C(OMOBJ *object);
void fn_1_5A84(OMOBJ *object);
void fn_1_5F5C(OMOBJ *object);
void fn_1_6268(OMOBJ *object);
void fn_1_645C(OMOBJ *object);
void fn_1_64B0(OMOBJ *object);
void fn_1_65F0(OMOBJ *object);
void fn_1_6788(OMOBJ *object);
void fn_1_68A0(OMOBJ *object);

/* Updates one player's Cashapult state and dispatches its active action on each player callback. */
void fn_1_68F0(OMOBJ *object)
{
    M642PlayerWork *work;
    MGACTOR *actor;
    M642ScoreRecord *teamRecord;

    work = object->data;
    actor = work->player->actor;
    if (work->state != 0) {
        MgActorPosGet(work->player->actor, &work->position);
    }

    teamRecord = &lbl_1_bss_2C->players[work->team];
    if (work->computer != 0 && work->aiParams &&
        *work->aiParams >= 0 &&
        teamRecord->value >= *work->aiParams) {
        lbl_1_bss_2C->active[work->team] = 1;
    }

    switch (work->state) {
    case 1: fn_1_5564(object); break;
    case 2: fn_1_577C(object); break;
    case 3: fn_1_5A84(object); break;
    case 4: fn_1_5F5C(object); break;
    case 5: fn_1_6268(object); break;
    case 6: fn_1_645C(object); break;
    case 7: fn_1_64B0(object); break;
    case 8: fn_1_65F0(object); break;
    case 9: fn_1_6788(object); break;
    case 10: fn_1_68A0(object); break;
    case 0: break;
    }
}

extern BOOL fn_1_1660(s32 team, Vec *position, Vec *target, u32 flags);
int abs(int value);

/* Plans a computer player's horizontal jump direction and steering time during its AI update. */
void fn_1_6A74(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec actorPosition;
    Vec referencePosition;
    Vec targetPosition;
    Vec displacement;
    f32 horizontalDistance;
    f32 apexFrames;
    f32 accelerationFrames;
    f32 cruiseFrames;
    f32 accelerationDistance;
    s32 heightBand;
    u32 targetFlags;

    MgActorPosGet(work->player->actor, &actorPosition);
    work->nextStep.x = 0.0f;
    work->nextStep.y = 0.0f;
    referencePosition.x = actorPosition.x;
    /* The first target search always starts at the top grid row. */
    referencePosition.y = 5400.0f;
    referencePosition.z = 0.0f;
    heightBand = (s32)((5400.0f - referencePosition.y) / 200.0f);
    heightBand = abs(heightBand);
    if (heightBand < 9) {
        if (((M642SceneWork *)lbl_1_bss_2C)->active[work->team] == 0) {
            targetFlags = 1;
        if (work->computer != 0 && work->aiParams && *work->aiParams == -1) {
                targetFlags |= 4;
            }
            if (!fn_1_1660(work->team, &referencePosition, &targetPosition, targetFlags)) {
                return;
            }
            if (work->jumps == 0) {
                work->savedTarget = targetPosition;
            }
        } else if (work->jumps == 0) {
            if (!fn_1_1660(work->team, &referencePosition, &targetPosition, 2)) {
                return;
            }
            if (work->jumps == 0) {
                work->savedTarget = targetPosition;
            }
        } else {
            targetPosition = work->savedTarget;
        }

        PSVECSubtract(&targetPosition, &actorPosition, &displacement);
        horizontalDistance = fabs(displacement.x);
        apexFrames = fabs(work->velocity.y / work->gravity);
        if (apexFrames < 23.333332061767578f) {
            accelerationFrames = apexFrames;
            cruiseFrames = 0.0f;
        } else {
            accelerationFrames = 23.333332061767578f;
            accelerationDistance = 7.0f * accelerationFrames / 2;
            cruiseFrames = (horizontalDistance - accelerationDistance) / 7.0f;
        }
        if (apexFrames > accelerationFrames + cruiseFrames) {
        work->steeringFrames = (s32)((accelerationFrames + cruiseFrames) - accelerationFrames / 2);
            work->flags |= (1 << 11);
        } else {
        work->steeringFrames = (s32)(accelerationFrames + cruiseFrames);
            work->flags &= ~(1 << 11);
        }
        work->aiFrames = (s16)apexFrames;
        if (displacement.x < 0.0f) {
            work->nextStep.x = -1.0f;
            work->nextStep.z = 0.0f;
        }
        if (displacement.x > 0.0f) {
            work->nextStep.x = 1.0f;
            work->nextStep.z = 0.0f;
        }
        /* The initial plan always enables the later steering reversal. */
        work->flags &= ~(1 << 11);
        work->flags |= (1 << 11);
        work->aiState = 1;
    }
}

/* CPU flight callback: holds its steering direction until the timer ends, then reverses it when
 * requested before advancing to the braking phase. */
void fn_1_6E64(OMOBJ *object)
{
    M642PlayerWork *work = object->data;

    work->direction = work->nextStep;
    if ((f64)work->steeringFrames > 0.0) {
        work->steeringFrames--;
        return;
    }
    if (work->flags & 0x800) {
        if (work->nextStep.x < 0.0f) {
            work->nextStep.x = 1.0f;
            work->nextStep.z = 0.0f;
        } else if (work->nextStep.x > 0.0f) {
            work->nextStep.x = -1.0f;
            work->nextStep.z = 0.0f;
        }
    }
    work->aiState = 2;
}

/* CPU braking callback: continues steering and, when reversing, waits for horizontal speed to fall
 * below 0.3 before allowing another target search. */
void fn_1_6F6C(OMOBJ *object)
{
    M642PlayerWork *work;
    work = object->data;
    work->direction = work->nextStep;
    if (work->flags & 0x800) {
        if (fabs(work->velocity.x) < 0.30000001192092896) {
            work->nextStep.x = 0.0f;
            work->nextStep.z = 1.0f;
            work->aiState = 3;
        }
    } else {
        work->aiState = 3;
    }
}

extern BOOL fn_1_1BC8(s32 team, Vec *position, Vec *target);
int abs(int value);

/* Chooses and times the next computer jump while its player is descending and ready to plan. */
void fn_1_7018(OMOBJ *object)
{
    M642PlayerWork *work = object->data;
    Vec actorPosition;
    Vec position;
    Vec target;
    Vec displacement;
    f32 horizontalDistance;
    f32 verticalDistance;
    f32 halfGravity;
    f32 cruiseFrames;
    f32 initialVelocity;
    f32 rootTerm;
    f32 firstFlightTime;
    f32 secondFlightTime;
    f32 flightFrames;
    f32 accelerationFrames;
    u32 targetFlags;
    s32 heightBand;
    f32 accelerationDistance;

    if (work->velocity.y > 0.0f) {
        return;
    }
    if (work->aiFrames > 0) {
        return;
    }
    MgActorPosGet(work->player->actor, &actorPosition);
    work->nextStep.x = 0.0f;
    work->nextStep.z = 1.0f;
    heightBand = (s32)((5400.0f - actorPosition.y) / 200.0f);
    heightBand = abs(heightBand);
    if (heightBand >= 9) {
        return;
    }

    if (((M642SceneWork *)lbl_1_bss_2C)->active[work->team] == 0) {
        targetFlags = 0;
        if (work->computer != 0 && work->aiParams && *work->aiParams == -1) {
            targetFlags = 4;
        }
        if (!fn_1_1660(work->team, &actorPosition, &target, targetFlags)) {
            return;
        }
    } else if ((work->flags & 0x1000) == 0) {
        position = actorPosition;
        if (!fn_1_1BC8(work->team, &position, &target)) {
            return;
        }
    } else {
        return;
    }

    PSVECSubtract(&target, &actorPosition, &displacement);
    horizontalDistance = (f32)fabs(displacement.x);
    halfGravity = work->gravity / 2;
    initialVelocity = work->velocity.y;
    verticalDistance = (f32)fabs(displacement.y);
    rootTerm = sqrtf((initialVelocity * initialVelocity) -
                             (4.0f * halfGravity * verticalDistance));
    firstFlightTime = halfGravity * ((-initialVelocity - rootTerm) / 2);
    secondFlightTime = halfGravity * ((-initialVelocity + rootTerm) / 2);
    if (firstFlightTime > secondFlightTime) {
        flightFrames = firstFlightTime;
    } else {
        flightFrames = secondFlightTime;
    }
    if (flightFrames < 23.333332061767578f) {
        accelerationFrames = flightFrames;
        cruiseFrames = 0.0f;
    } else {
        accelerationFrames = 23.333332061767578f;
        accelerationDistance = 7.0f * accelerationFrames / 2;
        cruiseFrames = (horizontalDistance - accelerationDistance) / 7.0f;
    }
    if (flightFrames >= accelerationFrames + cruiseFrames) {
        work->steeringFrames = (s32)((accelerationFrames + cruiseFrames) - accelerationFrames / 2);
    } else {
        work->steeringFrames = (s32)(accelerationFrames + cruiseFrames);
    }
    work->aiFrames = (s16)flightFrames;
    if (work->steeringFrames > 10) {
        work->flags |= (1 << 11);
    } else {
        work->flags &= ~(1 << 11);
    }
    if (fabs(displacement.x) >= 40.0) {
        if (displacement.x < 0.0f) {
            work->nextStep.x = -1.0f;
            work->nextStep.z = 0.0f;
        }
        if (displacement.x > 0.0f) {
            work->nextStep.x = 1.0f;
            work->nextStep.z = 0.0f;
        }
    }
    work->aiState = 1;
}

extern void fn_1_6A74(OMOBJ *obj);
extern void fn_1_6E64(OMOBJ *obj);
extern void fn_1_6F6C(OMOBJ *obj);
extern void fn_1_7018(OMOBJ *obj);

/* Advances a computer player's jump plan by dispatching its current planning phase. */
void fn_1_74B4(OMOBJ *object)
{
    s16 *playerState = object->data;

    if (playerState[54] != 0) {
        playerState[54]--;
    }
    switch (playerState[53]) {
        case 0:
            fn_1_6A74(object);
            break;
        case 1:
            fn_1_6E64(object);
            break;
        case 2:
            fn_1_6F6C(object);
            break;
        case 3:
            fn_1_7018(object);
            break;
    }
}

typedef struct M667Pool M667Pool;

extern void fn_1_1564(M642EffectRecord *data);
extern void fn_1_7708(OMOBJ *object);

/* Creates the reusable Cashapult effect pool and prepares each model during effect setup. */
void fn_1_7558(OMOBJ *object)
{
    M642EffectWork *work = object->data;
    M642EffectRecord *effect;
    M642EffectRecord unusedInitialEffect = {0};
    s32 model, modelIndex;

    work->pool = fn_1_8618(work->poolStorage, sizeof(work->poolStorage), sizeof(M642EffectRecord));
    for (modelIndex = 0; modelIndex < 30; modelIndex++) {
        effect = fn_1_86B0(work->pool);
        if (effect == NULL) {
            return;
        }
        model = lbl_1_bss_2C->scene_objects[7]->mdlId[lbl_1_bss_18->effectModels + 1];
        effect->type = 0;
        effect->model = model;
        effect->update = fn_1_1564;
        effect->tracks = lbl_1_data_3C0;
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(model, 3);
        Hu3DModelScaleSet(model, 0.0f, 0.0f, 0.0f);
        lbl_1_bss_18->effectModels++;
    }
    fn_1_140(object, fn_1_7708);
}

typedef struct M667Pool M667Pool;

/* Updates active Cashapult effects once per object-manager tick. */
void fn_1_7708(OMOBJ *object)
{
    M642EffectWork *work = object->data;
    M642EffectRecord *effect;
    effect = fn_1_8794(work->pool);
    while (effect) {
        if ((effect->flags & 1) && effect->update) {
            effect->update(effect);
            effect->frame++;
        }
        effect = fn_1_87D0(work->pool, effect);
    }
}
