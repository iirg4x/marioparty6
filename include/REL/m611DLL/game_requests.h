#ifndef REL_M611DLL_GAME_REQUESTS_H
#define REL_M611DLL_GAME_REQUESTS_H

// Acknowledges a scene request.
inline void acknowledgeSceneRequest(int &flags, unsigned long request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
}
#endif
