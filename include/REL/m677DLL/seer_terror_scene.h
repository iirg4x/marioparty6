// Creates the board resources. BoardView selects the weighted event and its result message
// before the opening wipe, then creates the ropes and waits for the player to choose one.

#ifndef REL_SEER_TERROR_SCENE_H
#define REL_SEER_TERROR_SCENE_H

#include "REL/framework/singleton.h"

// Selects the event records and resources for the board.
class SeerTerrorScene : public MinigameSingleton<SeerTerrorScene> {
public:
    SeerTerrorScene();
    ~SeerTerrorScene();

    void createCamera();
    void createMap();
    void createHostCharacter();
    void createPlayer();
    void createRopes();
    unsigned long startSelectedEvent(int, int);
    void loadParticleAnimations();
};

#endif
