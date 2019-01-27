#ifndef LIBPAS_ACP_MAIN_H
#define LIBPAS_ACP_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>

#include "app.h"
#include "cmd.h"
#include "prog.h"
#include "../dstructure_auto.h"
#include "../app.h"
#include "../timef.h"
#include "../util.h"
#include "../udp.h"
#include "../crc.h"
#include "../lcorrection.h"
#include "../lreduction.h"
#include "../filter/common.h"

#define ACP_RETRY_NUM 12

#define ACP_COMMAND_MAX_SIZE 16
#define ACP_DATA_MAX_SIZE 456
#define ACP_BUFFER_MAX_SIZE 508

#define ACP_NUM_STR_LENGTH 12

#define ACP_RESPONSE_BUF_SIZE_MIN 9UL
#define ACP_REQUEST_BUF_SIZE_MIN 7UL

#define ACP_LAST_PACK 0
#define ACP_MIDDLE_PACK 1

#define ACP_DELIMITER_COLUMN  '\t'
#define ACP_DELIMITER_COLUMN_STR  "\t"
#define ACP_DELIMITER_ROW  '\n'
#define ACP_DELIMITER_ROW_STR  "\n"
#define ACP_DELIMITER_BLOCK  '\r'
#define ACP_DELIMITER_BLOCK_STR "\r"
#define ACP_DELIMITER_PACKET  '\0'

//the same for request and response
#define ACP_BLOCK_IND_CRC 3

#define ACP_FLOAT_FORMAT_OUT "%.3f"
#define ACP_FLOAT_FORMAT_IN "%lf"

#define ACP_SEND_STR(V) acp_responseSendStr(V, ACP_MIDDLE_PACK, response, peer);

#define ACP_PEER_ID_LENGTH  16
#define ACP_PEER_ADDR_STR_LENGTH  32

typedef struct {
    char id[ACP_PEER_ID_LENGTH];
    char addr_str[ACP_PEER_ADDR_STR_LENGTH];
    int port;
    int *fd;
    struct sockaddr_in addr;
    socklen_t addr_size;
    int active;
    struct timespec time1;
} Peer;

DEC_LIST(Peer)
extern int initPeer(Peer * item, int *fd);
extern int initPeerList(PeerList * item, int *fd);
extern int cpPeer ( Peer *dest, const Peer * src );


typedef struct {
    char cmd[ACP_COMMAND_MAX_SIZE];
    char data[ACP_DATA_MAX_SIZE];
    char buf[ACP_BUFFER_MAX_SIZE];
    unsigned int id;
    size_t cmd_size;
    size_t data_size;
    size_t buf_size;
    size_t data_rows_count;
    uint8_t crc;
} ACPRequest;

typedef struct {
    char data[ACP_DATA_MAX_SIZE];
    char buf[ACP_BUFFER_MAX_SIZE];
    unsigned int id;
    unsigned int seq;
    int is_not_last;
    size_t data_size;
    size_t buf_size;
    uint8_t crc;
    int last_is_ok; //last packet has been successfully sent (multi-response)
} ACPResponse;

typedef int I1;

DEC_LIST(I1)

typedef struct {
    int p0;
    int p1;
} I2;

DEC_LIST(I2)

typedef struct {
    int p0;
    int p1;
    int p2;
} I3;
DEC_LIST(I3)

typedef double F1;
DEC_LIST(F1)

typedef double D1;
DEC_LIST(D1)

typedef struct {
    int p0;
    double p1;
} I1F1;

DEC_LIST(I1F1)

typedef struct {
    int p0;
    uint32_t p1;
} I1U321;
DEC_LIST(I1U321)

typedef char S1;

DEC_LIST(S1)

typedef struct {
    int p0;
    char p1[LINE_SIZE];
} I1S1;

DEC_LIST(I1S1)

typedef struct {
    char p0[LINE_SIZE];
    char p1[LINE_SIZE];
} S2;

DEC_LIST(S2)

typedef struct {
    int id;
    double value;
    struct timespec tm;
    int state;
} FTS;

DEC_LIST(FTS)

typedef struct {
    int id;
    int value;
    struct timespec tm;
    int state;
} ITS;

DEC_LIST(ITS)

//remote channel
typedef struct {
    int id;
    int channel_id;
    Peer peer;
} RChannel;

DEC_LIST(RChannel)

extern int cpRChannel(RChannel *dest, const RChannel *src);
extern int getRChannelFromList ( RChannel *dest , const RChannelList *list, int id );
extern int initRChannel ( RChannel* item, int *fd );

typedef struct {
    RChannel remote_channel;
    ITS input;
    struct timespec last_read_time;
    struct timespec interval_min;
    int last_return;
} SensorITS;

DEC_LIST(SensorITS)

typedef struct {
    RChannel remote_channel;
    FTS input;
    struct timespec last_read_time;
    struct timespec interval_min;
    int last_return;
} SensorFTS;

DEC_LIST(SensorFTS)

typedef struct {
    RChannel remote_channel;
    double last_output; //we will keep last output value in order not to repeat the same queries to peers
    double output_min; //max duty cycle value (see lib/pid.h PWM_RSL)
    double output_max;
} EM; //executive mechanism
DEC_LIST(EM)

#define FUN_ACP_REQUEST_DATA_TO(T) void acp_requestDataTo ## T(ACPRequest *request, T *list){\
acp_dataTo ## T(request->data, list);\
}
#define DEC_FUN_ACP_REQUEST_DATA_TO(T) extern void acp_requestDataTo ## T(ACPRequest *request, T *list);

#define FUN_ACP_RESPONSE_READ(T) int acp_responseRead ## T(T *list, ACPRequest *request, Peer *peer) {ACP_RESPONSE_CREATE if (!acp_responseRead(&response, peer)) {return 0;}if(!acp_responseCheck(&response, request)) {return 0;}acp_dataTo ## T(response.data, list);    return 1;}
#define DEC_FUN_ACP_RESPONSE_READ(T) extern int acp_responseRead ## T(T *list, ACPRequest *request, Peer *peer);


#define ACP_CMD_IS(V) acp_cmdcmp(&request, V)
#define ACP_REQUEST_CREATE ACPRequest request; acp_requestInit(&request);
#define ACP_RESPONSE_CREATE ACPResponse response; acp_responseInit(&response);

extern int acp_responseStrCat(ACPResponse *item, const char *str);

extern int acp_requestStrCat(ACPRequest *item, const char *str);

extern void acp_responseInit(ACPResponse *item);

extern void acp_requestInit(ACPRequest *item);

extern int acp_responseCoopRequest(ACPResponse *response, ACPRequest *request);

extern int acp_requestRead(ACPRequest *item, Peer *peer);

extern int acp_responseRead(ACPResponse *item, Peer *peer);

extern int acp_requestCheck(ACPRequest *item);

extern int acp_responseCheck(ACPResponse *response, ACPRequest *request);

extern void acp_requestSetCmd(ACPRequest * item, const char *cmd);

extern void acp_responsePack(ACPResponse *item);

extern void acp_requestPack(ACPRequest *item);

extern int acp_responseSend(ACPResponse *response, Peer *peer);

extern int acp_requestSend(ACPRequest *request, Peer *peer);

extern int acp_requestSendCmd(const char *cmd, ACPRequest *request, Peer *peer);

extern int acp_requestSendUnrequitedCmd(const char *cmd, Peer *peer);

extern int acp_requestSendI1List(char *cmd, const I1List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendI1F1List(char *cmd, const I1F1List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendI1U321List(char *cmd, const I1U321List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendI2List(char *cmd, const I2List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendS2List(char *cmd, const S2List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendS1List(char *cmd, const S1List *data, ACPRequest *request, Peer *peer);

extern int acp_requestSendUnrequitedI1List(char *cmd, const I1List *data, Peer *peer);

extern int acp_requestSendUnrequitedI1F1List(char *cmd, const I1F1List *data, Peer *peer);

extern int acp_requestSendUnrequitedI1U321List(char *cmd, const I1U321List *data, Peer *peer);

extern int acp_requestSendUnrequitedI2List(char *cmd, const I2List *data, Peer *peer);

extern int acp_requestSendUnrequitedS2List(char *cmd, const S2List *data, Peer *peer);

extern int acp_requestSendUnrequitedS1List(char *cmd, const S1List *data, Peer *peer);

extern void acp_responseSendStr(const char *s, int is_not_last, ACPResponse *response, Peer *peer);

DEC_FUN_ACP_RESPONSE_READ(I1List)

DEC_FUN_ACP_RESPONSE_READ(I2List)

DEC_FUN_ACP_RESPONSE_READ(I1F1List)

DEC_FUN_ACP_RESPONSE_READ(I1U321List)

DEC_FUN_ACP_RESPONSE_READ(FTSList)

DEC_FUN_ACP_RESPONSE_READ(ITSList)

extern int acp_setRChannelFloat(RChannel *rchannel, double output);

extern int acp_setRChannelInt(RChannel *rchannel, int output);

extern int acp_getRChannelITS ( ITS *output, RChannel *rchannel );

extern int acp_getRChannelFTS ( FTS *output, RChannel *rchannel );

extern int acp_getFTS(FTS *output, Peer *peer, int remote_channel_id);

extern int acp_getITS(ITS *output, Peer *peer, int remote_channel_id);

extern int acp_getError(uint32_t *output, Peer *peer, int remote_channel_id);

extern int acp_getProgEnabled(Peer *peer, int remote_channel_id);

extern int acp_peerItemSendCmd(Peer *peer, int remote_channel_id, char *cmd);

extern void acp_pingPeer(Peer *item);

extern void acp_pingPeerList(PeerList *list, struct timespec interval, struct timespec now);

extern int acp_peerListIsActive(PeerList *list);

extern int acp_responseSendCurTime(ACPResponse *item, Peer *peer);

extern int acp_sendCmdGetInt(Peer *peer, char* cmd, int *output);

extern int acp_sendCmdGetFloat(Peer *peer, char* cmd, double *output);

extern int acp_responseFTSCat(int id, double value, struct timespec tm, int state, ACPResponse *response);

extern int acp_responseITSCat(int id, int value, struct timespec tm, int state, ACPResponse *response);

extern int acp_cmdcmp(ACPRequest *request, char * cmd);

extern void acp_printI1(I1List *list);

extern void acp_printI2(I2List *list);

extern void acp_printI3(I3List *list);

extern void acp_sendPeerListInfo(PeerList *pl, ACPResponse *response, Peer *peer);

extern void acp_sendLCorrectionListInfo(LCorrectionList *list, ACPResponse *response, Peer *peer);

extern void acp_sendLReductionListInfo(LReductionList *list, ACPResponse *response, Peer *peer);

extern void acp_sendFilterListInfo(FilterList *list, ACPResponse *response, Peer *peer );

DEC_FUN_ACP_REQUEST_DATA_TO(I1List)


DEC_FUN_ACP_REQUEST_DATA_TO(I2List)


DEC_FUN_ACP_REQUEST_DATA_TO(I3List)


DEC_FUN_ACP_REQUEST_DATA_TO(F1List)


DEC_FUN_ACP_REQUEST_DATA_TO(I1F1List)

DEC_FUN_ACP_REQUEST_DATA_TO(I1U321List)

DEC_FUN_ACP_REQUEST_DATA_TO(S1List)


DEC_FUN_ACP_REQUEST_DATA_TO(I1S1List)


DEC_FUN_ACP_REQUEST_DATA_TO(FTSList)

DEC_FUN_ACP_REQUEST_DATA_TO(ITSList)


DEC_FUN_ACP_REQUEST_DATA_TO(S2List)

#endif 

