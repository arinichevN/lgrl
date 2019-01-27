#include "mobile.h"
#include "../app.h"

int acp_sendSMS(Peer *peer, const char *phone, const char *message) {
    S2 di[1];
    strncpy(di[0].p0, phone, strlen(phone)+1);
    strncpy(di[0].p1, message, strlen(message)+1);
    S2List data = {.item = di, .length = 1, .max_length = 1};
    if (!acp_requestSendUnrequitedS2List(ACP_CMD_MOBILE_SEND_SMS, &data, peer)) {
        printde("failed to send request where peer.id = %s\n", peer->id);
        return 0;
    }
    return 1;
}

int acp_makeCall(Peer *peer, const char *phone) {
    S1 di[LINE_SIZE];
    strncpy(di, phone, strlen(phone)+1);
    S1List data = {.item = di, .length = 1, .max_length = 1};
    if (!acp_requestSendUnrequitedS1List(ACP_CMD_MOBILE_RING, &data, peer)) {
        printde("failed to send request where peer.id = %s\n", peer->id);
        return 0;
    }
    return 1;
}

