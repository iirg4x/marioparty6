// Scene lifetime, player setup and terrain queries.

extern "C" {
#include "game/board/player.h"
#include "game/wipe.h"
#include "game/audio.h"
#include "msm_stream.h"
}

#include "REL/framework/frame_durations.h"
#include "REL/framework/model.h"
#include "REL/framework/chara_motion.h"
#include "REL/framework/scene_shadow.h"
#include "REL/framework/list_object.h"
#include <memory>
#include "REL/m623DLL/scene_collision.h"
#include "REL/m623DLL/player_table_1vs3.h"
#include "REL/m623DLL/player_state.h"
#include "REL/m623DLL/scene_model_owner.h"
#include "REL/m623DLL/item_effects_owner.h"
#include "REL/m623DLL/particle_animations.h"
#include "REL/m623DLL/score_owner.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m623DLL/circle_renderer.h"
#include "REL/m623DLL/scene_camera_owner.h"
#include "REL/m623DLL/scene_camera_api.h"
#include "REL/framework/tri_colli.h"
#include "REL/m623DLL/scene_storage.h"
#include "REL/framework/frame_time.h"
#include <new>
#include "REL/framework/sound.h"
#include "REL/framework/assertion.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
#include "REL/m623DLL/scene_object.h"
#include "REL/framework/minigame_core.h"
#include "REL/framework/octtree_man.h"
#include "REL/framework/hu_archive.h"
#include "REL/framework/singleton.h"
#include "REL/framework/thread.h"
#include "REL/framework/category_sphere.h"
#include "datadir_enum.h"

// Shadow projection shared by the scene.
SceneShadow *lbl_1_bss_24;
// The lone player.
PlayerState *lbl_1_bss_28;
// The three players on the opposing team.
PlayerState *lbl_1_bss_2C[3];
// The timer loop tests this flag after clearing it.
unsigned char lbl_1_bss_38;
// Elapsed frames during play.
s32 lbl_1_bss_3C;
// Four unread bytes of scene storage.
unsigned char lbl_1_bss_40[4];
// The lone player's current Y position.
f32 lbl_1_bss_44;
// The lone player's current Z position.
f32 lbl_1_bss_48;
// Collision query for the course.
TriangleCollision *lbl_1_bss_4C;
// Player slot order, with the lone player first.
int lbl_1_bss_50[4];
// Character models for the finish sequence.
PlayerConfigSelection *lbl_1_bss_60[4];
// Character motion files for the finish sequence.
const unsigned int lbl_1_rodata_28[2] = { DATANUM(0x930000, 38), DATANUM(0x930000, 7) };
// Radius assigned to each finish character's interaction sphere.
const f32 lbl_1_rodata_30[] = { 200.0f };

// Holds shared events, registered requests and the scene collision octree.
class SceneState : public MinigameSingleton<SceneState> {
  public:
    EventManager eventManager;
    GameObjectManager gameObjects;
    Octree geometry;
    CircleRenderer circles;
    SceneState();
    // Returns the scene collision tree.
    Octree *tree()
    {
        return &geometry;
    }
};

// Loads the scene geometry archive.
SceneState::SceneState() : geometry(new HuArchive(DATANUM(0x520000, 34)))
{
}

// Creates the scene thread and its shared state.
SceneObject::SceneObject() : ThreadEntry(0x8000), state(new SceneState)
{
}

// Deletes the scene shadow and collision query, and releases the owned state.
SceneObject::~SceneObject()
{
    delete lbl_1_bss_24;
    delete lbl_1_bss_4C;
}

// Allocates the four player states and registers their post-update hook.
void fn_1_774(void)
{
    short nightMode;
    bool dayMode;
    fn_1_E5DC(lbl_1_bss_50);
    nightMode = lbl_802C0278;
    dayMode = nightMode == 0;

    lbl_1_bss_28 = new PlayerState(lbl_1_bss_50[0], dayMode, 0);
    lbl_1_bss_2C[0] = new PlayerState(lbl_1_bss_50[1], dayMode, 1);
    lbl_1_bss_2C[1] = new PlayerState(lbl_1_bss_50[2], dayMode, 2);
    lbl_1_bss_2C[2] = new PlayerState(lbl_1_bss_50[3], dayMode, 3);
    setPostUpdateHook(1, &fn_1_9D60);
}

// Deletes the four player states and clears their stored pointers.
void fn_1_8D4(void)
{
    delete lbl_1_bss_28;
    delete lbl_1_bss_2C[0];
    delete lbl_1_bss_2C[1];
    delete lbl_1_bss_2C[2];

    lbl_1_bss_28 = 0;
    lbl_1_bss_2C[0] = 0;
    lbl_1_bss_2C[1] = 0;
    lbl_1_bss_2C[2] = 0;
}

// Creates the character models and registers their scaled interaction spheres.
void fn_1_A10(void)
{
    fn_8005BB8C(-1);
    for (int index = 0; index < 4; ++index) {
        int character = lbl_1_bss_50[index];
        PlayerConfigSelection *finishCharacter = new PlayerConfigSelection(character, 2);
        createCharacterMotions(&static_cast<MotionOwner &>(*finishCharacter), lbl_1_rodata_28, 2);
        fn_1_C89C(index, *finishCharacter);
        lbl_1_bss_60[index] = finishCharacter;
        RegisteredValue *interactionSphere = new RegisteredValue(0);
        const f32 radius(lbl_1_rodata_30[0]);
        interactionSphere->setRadius(radius);
        finishCharacter->getPosition(&interactionSphere->center);
    }
}

// Creates the shared scene objects and starts the scene setup.
void SceneObject::setupObjects(OMOBJMAN *)
{
    GXColor lightColor;
    short nightMode;
    Vec lightPosition;

    nightMode = lbl_802C0278;
    bool dayMode = !nightMode;
    enableDefaultAttributeA();
    Vec lightDirection;
    RandomSeed(OSGetTick());
    new SceneCamera();
    new SceneModel(dayMode);
    new ParticleAnimations();
    new ItemEffectsOwner(RandomInteger(5));
    new MGScoreOwner();
    fn_1_774();
    fn_800297D4(0, &lightPosition, &lightDirection, &lightColor);
    lightDirection.x = 0.5f;
    lightDirection.y = -1.0f;
    lightDirection.z = 0.0f;
    lbl_1_bss_24 = new SceneShadow(3, &lightDirection, 5.0f, 100);
    lbl_1_bss_24->setCategory(0, 1);
    lbl_1_bss_24->setCategory(1, 2);
    if (dayMode) {
        lbl_1_bss_24->setColor(0x20, 0x20, 0x80, 0x96);
        return;
    }
    lbl_1_bss_24->setColor(0x10, 0x10, 0x50, 0x46);
}

// Resets players and effects, restores the camera and changes the light direction.
void fn_1_E5C(void)
{
    fn_1_8D4();
    delete ParticleAnimations::current();
    delete ItemEffectsOwner::current();
    Model *stageModel = fn_1_2C50();
    fn_1_C1C4(stageModel);
    fn_1_A10();
    Vec lightPosition;
    Vec lightDirection;
    GXColor lightColor;
    fn_800297D4(0, &lightPosition, &lightDirection, &lightColor);
    lightDirection.x = 0.0f;
    lightDirection.y = -1.0f;
    lightDirection.z = 0.3f;
    lbl_1_bss_24->setDirection(&lightDirection);
}

// Leaves scene state unchanged for incoming event packets.
void SceneObject::receive(const EventPacket *)
{
}

// Resumes the scene thread, dispatches events and updates scene and request objects.
void SceneObject::updateGame()
{
    ThreadEntry::resume(0);
    process_event_queue();
    updateGameObjects();
    updateRequestObjects();
    if (currentRequest() >= 2)
        lbl_1_bss_3C++;
}

// Runs the introduction, game and finish sequence on the scene thread.
void SceneObject::run(void *)
{
    lbl_1_bss_3C = 0;
    s32 sound = soundPlay(0x6F8);
    postRequest(0);
    MinigameCore::wipeIn();
    for (s32 i = 0; i < secondsToFrames(3L); ++i)
        suspend(0);
    postRequest(1);
    MinigameCore::showStart();
    s32 stream = HuAudSStreamPlay(MSM_STREAM_RELAXED);
    for (s32 i = 0; i < secondsToFrames(2.0); ++i)
        suspend(0);
    fn_1_30A8();
    while (!MinigameCore::messageDone())
        suspend(0);

    // This local is never read.
    unsigned char initialFlag = 0;
    postRequest(2);
    // Waits for the flag or for the countdown to expire.
    lbl_1_bss_38 = 0;
    s32 finishDelay = secondsToFrames(20L);
    while (lbl_1_bss_38 == 0) {
        suspend(0);
        if (--finishDelay < 0)
            break;
    }
    postRequest(3);
    HuAudSStreamFadeOut(stream, 100);
    MinigameCore::showFinish();
    for (s32 i = 0; i < secondsToFrames(2L); ++i)
        suspend(0);
    WipeColorSet(0xFF, 0xFF, 0xFF);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    for (s32 i = 0; i < 60; ++i)
        suspend(0);
    fn_1_E5C();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
    for (s32 i = 0; i < 60; ++i)
        suspend(0);
    postRequest(4);
    s32 soloCoins = fn_1_8AAC(0);
    s32 teamCoins = fn_1_8AAC(1);
    HuAudJinglePlay(soloCoins + teamCoins == 0 ? MSM_STREAM_BAD_NEWS : MSM_STREAM_WINNING);
    for (s32 i = 0; i < 4; ++i) {
        GWMgCoinSet(lbl_1_bss_50[i], i == 0 ? soloCoins : teamCoins);
    }
    for (s32 i = 0; i < 4; ++i) {
        PlayerConfigSelection *character = lbl_1_bss_60[i];
        if (fn_1_8AAC(i == 0 ? 0 : 1) > 0)
            shiftCharacterMotion(&static_cast<MotionOwner &>(*character), DATANUM(0x930000, 38),
                                 10.0f, 0.0f, 0);
        else
            shiftCharacterMotion(&static_cast<MotionOwner &>(*character), DATANUM(0x930000, 7),
                                 10.0f, 0.0f, 0);
    }
    for (s32 i = 0; i < secondsToFrames(3L); ++i)
        suspend(0);
    while (!MinigameCore::messageDone())
        suspend(0);
    MinigameCore::requestExit();
    for (;;)
        suspend(reinterpret_cast<void *>(-1));
}

// Deletes the player states and all registered request objects.
void SceneObject::finishGame()
{
    fn_1_8D4();
    deleteRequestObjects();
}

// Returns the octree used by scene collision queries.
// Nothing in this minigame calls this.
Octree *fn_1_19B0(void)
{
    return SceneState::getChecked().tree();
}

// Allocates a collision query for the shared scene octree.
TriangleCollision *fn_1_1A2C()
{
    return new TriangleCollision(SceneState::getChecked().tree());
}

// Samples the terrain height at the supplied X and Z coordinates.
f32 fn_1_1ACC(f32 x, f32 z)
{
    f32 stepScale = -100.0f;
    Vector3 rayStart, rayEnd;
    if (lbl_1_bss_4C == 0) {
        lbl_1_bss_4C = fn_1_1A2C();
    }
    rayEnd.x = rayStart.x = x;
    rayEnd.z = rayStart.z = z;
    int index = 0;
    for (;;) {
        f32 step = stepScale * (f32) index;
        rayStart.y = 3500.0f + step;
        rayEnd.y = rayStart.y + (1.05f * stepScale);
        if (!(rayEnd.y < 240.0f)) {
            if (lbl_1_bss_4C->castRay(&rayStart, &rayEnd))
                break;
            ++index;
            continue;
        } else {
            return -1.0f;
        }
    }
    return rayStart.y + ((rayEnd.y - rayStart.y) * lbl_1_bss_4C->getMaximumDistance());
}

// Interpolates the moving X bound over the game timer.
static inline f32 movingXBound()
{
    f32 bound;
    f32 start;
    start = -4859.523f;
    bound = (f32) lbl_1_bss_3C / (f32) secondsToFrames(20L);
    bound = start + bound * (51731.28f - start);
    return bound;
}

// Returns the moving upper X bound at the current frame.
// Nothing in this minigame calls this.
f32 fn_1_1CD4(void)
{
    return movingXBound();
}

// Returns the moving upper X bound at the current frame.
f32 fn_1_1D98(void)
{
    return movingXBound();
}

// Returns the moving lower X bound at the current frame.
f32 fn_1_1E5C(void)
{
    return movingXBound() - 1000.0f;
}
