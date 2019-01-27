#include "dht22.h"

#define DHT22_MAXTIMINGS 85
#define DHT22_MAX_VAL 83

int dht22_read ( int pin, double *t, double *h ) {
    //sending request
    pinPUD ( pin, PUD_OFF );
    pinModeOut ( pin );
    pinHigh ( pin );
    DELAY_US_BUSY ( 250 );
    pinLow ( pin );
    DELAY_US_BUSY ( 500 );
    pinPUD ( pin, PUD_UP );
    DELAY_US_BUSY ( 15 );

    pinModeIn ( pin );

    //reading response from chip
    int i;
    int laststate = HIGH;
    int arr[DHT22_MAXTIMINGS]; //we will write signal duration here
    memset ( arr, 0, sizeof arr );

    for ( i = 0; i < DHT22_MAXTIMINGS; i++ ) {
        int c = 0;
        while ( pinRead ( pin ) == laststate ) {
            c++;
            DELAY_US_BUSY ( 1 );
            if ( c >= DHT22_MAX_VAL ) {
                printde ( "too long signal=%d where pin=%d\n", laststate, pin );
                return 0;
            }
        }
        laststate = pinRead ( pin );
        arr[i] = c;
    }

    //dealing with response from chip
    int j = 0; //bit counter
    uint8_t data[5] = {0, 0, 0, 0, 0};
    for ( i = 0; i < DHT22_MAXTIMINGS; i++ ) {
        if ( arr[i] > DHT22_MAX_VAL ) { //too long signal found
            break;
        }
        if ( i < 3 ) { //skip first 3 signals (response signal)
            continue;
        }
        if ( i % 2 == 0 ) { //dealing with data signal
            if ( j >= 40 ) { //we have 8*5 bits of data
                break;
            }
            data[j / 8] <<= 1; //put 0 bit
            if ( arr[i] > arr[i - 1] ) { //if (high signal duration > low signal duration) then 1
                data[j / 8] |= 1;
            }
            j++;
        }
    }

#ifdef MODE_DEBUG
    printf ( "dht22_read: data: %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx, j=%d\n", data[0], data[1], data[2], data[3], data[4], j );
    for ( i = 0; i < DHT22_MAXTIMINGS; i++ ) {
        printf ( "%d ", arr[i] );
    }
    puts ( "" );
#endif
    if ( j < 40 ) {
        printde ( "j=%d but j>=40 expected where pin=%d\n", j, pin );
        return 0;
    }
    if ( ( data[4] == ( ( data[0] + data[1] + data[2] + data[3] ) & 0xFF ) ) ) {
        *h = ( double ) data[0] * 256 + ( double ) data[1];
        *h /= 10;
        *t = ( double ) ( data[2] & 0x7F ) * 256 + ( double ) data[3];
        *t /= 10.0;
        if ( ( data[2] & 0x80 ) != 0 ) {
            *t *= -1;
        }
        return 1;
    }
    printde ( "bad crc where pin=%d\n", pin );
    return 0;

}

#define FOREACHSENSOR for(size_t sr=0; sr<length;sr++)
#define SENSOR_PIN pin[sr]
//parallel reading
void dht22_readp ( int *pin, double *t, double *h,int *success, size_t length ) {
    //sending request
    FOREACHSENSOR {
        pinPUD ( SENSOR_PIN, PUD_OFF );
        pinModeOut ( SENSOR_PIN );
        pinHigh ( SENSOR_PIN );
        success[sr]= 1;
    }
    DELAY_US_BUSY ( 250 );

    FOREACHSENSOR pinLow ( SENSOR_PIN );
    DELAY_US_BUSY ( 500 );

    FOREACHSENSOR pinPUD ( SENSOR_PIN, PUD_UP );
    DELAY_US_BUSY ( 15 );

    FOREACHSENSOR pinModeIn ( SENSOR_PIN );

    //reading response from chip
    int arr[length][DHT22_MAXTIMINGS]; //we will write signal duration here
    memset ( arr, 0, sizeof arr );
    int c[length];
    int i[length];
    int laststate[length];
    FOREACHSENSOR {c[sr]=0; i[sr]=0; laststate[sr] = HIGH;}
    while ( 1 ) {
        int done=1;
        FOREACHSENSOR {
            if ( i[sr]>=DHT22_MAXTIMINGS ) continue;
            if ( c[sr] >= DHT22_MAX_VAL ) {
                success[sr]= 0;
                continue;
            }
            done=0;
            int state=pinRead ( SENSOR_PIN );
            if ( state == laststate[sr] ) {
                c[sr]++;
            } else{
                laststate[sr] = state;
                arr[sr][i[sr]]=c[sr];
                c[sr]=0;
                i[sr]++;
            }
        }
        if ( done ) break;
        DELAY_US_BUSY ( 1 );
    }
    //check error while reading data
    FOREACHSENSOR {
        if ( !success[sr] ) {
            printde ( "too long signal where pin=%d\n",SENSOR_PIN );
        }
    }

    //dealing with response from chip
    FOREACHSENSOR {
		if ( !success[sr] ) continue;
        int j = 0; //bit counter
        uint8_t data[5] = {0, 0, 0, 0, 0};
        for ( int i = 0; i < DHT22_MAXTIMINGS; i++ ) {
            if ( arr[sr][i] > DHT22_MAX_VAL ) { //too long signal found
                break;
            }
            if ( i < 3 ) { //skip first 3 signals (response signal)
                continue;
            }
            if ( i % 2 == 0 ) { //dealing with data signal
                if ( j >= 40 ) { //we have 8*5 bits of data
                    break;
                }
                data[j / 8] <<= 1; //put 0 bit
                if ( arr[sr][i] > arr[sr][i - 1] ) { //if (high signal duration > low signal duration) then 1
                    data[j / 8] |= 1;
                }
                j++;
            }
        }

#ifdef MODE_DEBUG
        printf ( "dht22_read: data: %.2hhx %.2hhx %.2hhx %.2hhx %.2hhx, j=%d\n", data[0], data[1], data[2], data[3], data[4], j );
        for ( int i = 0; i < DHT22_MAXTIMINGS; i++ ) {
            printf ( "%d ", arr[sr][i] );
        }
        puts ( "" );
#endif
        if ( j < 40 ) {
            printde ( "j=%d but j>=40 expected where pin=%d\n", j, SENSOR_PIN );
            success[sr]= 0;
            break;
        }
        if ( data[4] == ( ( data[0] + data[1] + data[2] + data[3] ) & 0xFF ) ) {
            h[sr] = ( double ) data[0] * 256 + ( double ) data[1];
            h[sr] /= 10;
            t[sr] = ( double ) ( data[2] & 0x7F ) * 256 + ( double ) data[3];
            t[sr] /= 10.0;
            if ( ( data[2] & 0x80 ) != 0 ) {
                t[sr] *= -1;
            }
            success[sr]= 1;
            break;
        }
        printde ( "bad crc where pin=%d\n", SENSOR_PIN );
        success[sr]= 0;
    }


}
#undef FOREACHSENSOR
#undef SENSOR_PIN

