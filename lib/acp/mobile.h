
#ifndef LIBPAS_ACP_MOBILE_H
#define LIBPAS_ACP_MOBILE_H

#include "main.h"
#define ACP_CMD_MOBILE_RING "mring"
#define ACP_CMD_MOBILE_SEND_SMS "mssms"

extern int acp_sendSMS(Peer *peer, const char *phone, const char *message);

extern int acp_makeCall(Peer *peer, const char *phone);

#endif 

