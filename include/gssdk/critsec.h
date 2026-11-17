#ifndef GSSDK_CRITSEC_H
#define GSSDK_CRITSEC_H

struct CRITSEC_TAG;
long critsec_Enter(struct CRITSEC_TAG *const hCritSec);
long critsec_Leave(struct CRITSEC_TAG *const hCritSec);

#endif
