/* Critical section entry and exit for the voice recognizer. */
#include "gssdk/critsec.h"

/* Returns success without acquiring a lock on this platform. */
long critsec_Enter(struct CRITSEC_TAG *const hCritSec)
{
    return 0;
}

/* Returns success without releasing a lock on this platform. */
long critsec_Leave(struct CRITSEC_TAG *const hCritSec)
{
    return 0;
}
