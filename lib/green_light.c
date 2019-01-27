#include "green_light.h"
int greenLight_init ( GreenLight *item, int *fd ) {
    return initRChannel ( &item->remote_channel, fd );
}

int greenLight_isGreen ( GreenLight *item ) {
    if ( !item->active )  return 1;
    FTS result;
    if ( acp_getFTS ( &result, &item->remote_channel.peer, item->remote_channel.channel_id ) ) {
        if ( result.value == item->value ) {
            return 1;
        }
    }
    return 0;
}
