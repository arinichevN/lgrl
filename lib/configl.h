
#ifndef LIBPAS_CONFIG_H
#define LIBPAS_CONFIG_H

#include "dbl.h"
#include "acp/main.h"
#include "timef.h"
#include "green_light.h"


extern int config_getPeerList(PeerList *list, const char *db_path);

extern int config_getRChannel ( RChannel *item, int rchannel_id, sqlite3 *dbl, const char *db_path );

extern int config_getPeer(Peer *item, const char * peer_id, sqlite3 *db, const char *db_path);

extern int config_getPort(int *port, const char *peer_id, sqlite3 *dbl, const char *db_path);

extern int config_getGreenLight ( GreenLight *item, int green_light_id, sqlite3 *dbl, const char *db_path );

extern int config_getPhoneNumberListG(S1List *list, int group_id, const char *db_path);

extern int config_getPhoneNumberListO(S1List *list, const char *db_path);

#endif 

