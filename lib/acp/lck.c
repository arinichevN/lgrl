#include "lck.h"

void acp_lck_waitUnlock(Peer *peer, unsigned int interval_us) {
    int state = 1;
    while (1) {
        switch (state) {
            case 1:
                putsdo("sending ping request to locker...\n");
                acp_pingPeer(peer);
                if (peer->active) {
                    state = 2;
                }
                break;
            case 2:
                putsdo("sending unlock request to locker...\n");
                if (acp_requestSendUnrequitedCmd(ACP_CMD_LCK_UNLOCK, peer)) {
                    state = 3;
                }
                break;
            case 3:
            {
                putsdo("checking locker to be unlocked...\n");
                int locked = 1;
                if (acp_sendCmdGetInt(peer, ACP_CMD_GET_DATA, &locked)) {
                    if (!locked) {
                        state = 4;
                    }
                }
                break;
            }
            case 4:
                putsdo("done!\n");
                return;
                break;
        }
        delayUsIdle(interval_us);
    }
}

void acp_lck_lock(Peer *peer) {
    acp_requestSendUnrequitedCmd(ACP_CMD_LCK_LOCK, peer);
}