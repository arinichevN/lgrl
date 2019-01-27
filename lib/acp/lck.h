
#ifndef LIBPAS_ACP_LCK_H
#define LIBPAS_ACP_LCK_H

#include "main.h"

#define ACP_CMD_LCK_LOCK "lckl"
#define ACP_CMD_LCK_UNLOCK "lckul"

extern void acp_lck_waitUnlock(Peer *item, unsigned int interval_us);

extern void acp_lck_lock(Peer *item);

#endif 

