/* Critical section operations shared by the voice SDK. */
#ifndef GSSDK_CRITSEC_H
#define GSSDK_CRITSEC_H

/* Opaque handle accepted by the SDK critical-section API. */
struct CRITSEC_TAG;

/* Returns success from the platform enter operation without acquiring a lock. */
long critsec_Enter(struct CRITSEC_TAG *const hCritSec);

/* Returns success from the platform leave operation without releasing a lock. */
long critsec_Leave(struct CRITSEC_TAG *const hCritSec);

#endif
