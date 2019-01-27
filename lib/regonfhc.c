
#include "regonfhc.h"
#include "reg.c"
#include "green_light.c"

static void controlEM ( RegOnfHCEM *item, double output ) {
    if ( item->use ) {
        if ( output>item->output_max ) output=item->output_max;
        if ( output<item->output_min ) output=item->output_min;
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
    }
}

static void offEM ( RegOnfHCEM *item ) {
    int output=item->output_min;
    if ( item->use ) {
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
    }
}

static void onEM ( RegOnfHCEM *item ) {
    int output=item->output_max;
    if ( item->use ) {
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
    }
}


void regonfhc_control ( RegOnfHC *item ) {
    double output;
    switch ( item->state ) {
    case REG_INIT:
        if ( !greenLight_isGreen ( &item->green_light ) ) {
            printde ( "light is not green\n" );
            return;
        }
        if ( !reg_sensorRead(&item->sensor) ) {
            putsde ( "reading from sensor failed\n" );
            return;
        }
        ton_ts_reset ( &item->tmr );
        item->state_r = REG_HEATER;
        offEM ( &item->heater );
        offEM ( &item->cooler );
        item->snsrf_count = 0;
        item->state_onf = REG_WAIT;
        item->state = REG_BUSY;
        if ( item->heater.use && item->cooler.use ) {
            if ( SNSR_VAL > item->goal ) {
                item->state_r = REG_COOLER;
            } else {
                item->state_r = REG_HEATER;
            }
        } else if ( item->heater.use && !item->cooler.use ) {
            item->state_r = REG_HEATER;
        } else if ( !item->heater.use && item->cooler.use ) {
            item->state_r = REG_COOLER;
        } else {
            item->state = REG_DISABLE;
        }
        break;
    case REG_BUSY: {
        if ( !acp_getRChannelFTS ( &item->input, &item->sensor ) ) {
            item->snsrf_count = 0;
            int value_is_out = 0;
            char other_em;
            RegOnfHCEM *reg_em = NULL;
            RegOnfHCEM *reg_em_other = NULL;
            switch ( item->state_r ) {
            case REG_HEATER:
                if ( VAL_IS_OUT_H ) {
                    value_is_out = 1;
                }
                other_em = REG_COOLER;
                reg_em = &item->heater;
                reg_em_other = &item->cooler;
                break;
            case REG_COOLER:
                if ( VAL_IS_OUT_C ) {
                    value_is_out = 1;
                }
                other_em = REG_HEATER;
                reg_em = &item->cooler;
                reg_em_other = &item->heater;
                break;
            }
            if ( value_is_out && reg_em_other->use ) {
                if ( ton_ts ( item->change_gap, &item->tmr ) ) {
#ifdef MODE_DEBUG
                    char *state1 = reg_getStateStr ( item->state_r );
                    char *state2 = reg_getStateStr ( other_em );
                    printf ( "state_r switched from %s to %s\n", state1, state2 );
#endif
                    item->state_r = other_em;
                    offEM ( reg_em );
                }
            } else {
                ton_ts_reset ( &item->tmr );
            }
            switch ( item->state_r ) {
            case REG_HEATER:
                reg_em = &item->heater;
                reg_em_other = &item->cooler;
                break;
            case REG_COOLER:
                reg_em = &item->cooler;
                reg_em_other = &item->heater;
                break;
            }
            switch ( item->state_onf ) {
            case REG_DO:
                switch ( item->state_r ) {
                case REG_HEATER:
                    if ( SNSR_VAL > item->goal + item->heater.delta ) {
                        item->state_onf = REG_WAIT;
                    }
                    break;
                case REG_COOLER:
                    if ( SNSR_VAL < item->goal - item->cooler.delta ) {
                        item->state_onf = REG_WAIT;
                    }
                    break;
                }
                output = reg_em->output_max;
                break;
            case REG_WAIT:
                switch ( item->state_r ) {
                case REG_HEATER:
                    if ( SNSR_VAL < item->goal - item->heater.delta ) {
                        item->state_onf = REG_DO;
                    }
                    break;
                case REG_COOLER:
                    if ( SNSR_VAL > item->goal + item->cooler.delta ) {
                        item->state_onf = REG_DO;
                    }
                    break;
                }
                output = reg_em->output_min;
                break;
            }
            controlEM ( reg_em, output );
            offEM ( reg_em_other );
            if ( reg_secureNeed ( &item->secure_out ) ) {
                item->state = REG_SECURE;
            }
        } else {
            if ( item->snsrf_count > SNSRF_COUNT_MAX ) {
                offEM ( &item->heater );
                offEM ( &item->cooler );
                item->state = REG_INIT;
                putsde ( "reading from sensor failed, EM turned off\n" );
            } else {
                item->snsrf_count++;
                printde ( "sensor failure counter: %d\n", item->snsrf_count );
            }
        }
        break;
    }
    case REG_SECURE:
        controlEM ( &item->heater, item->secure_out.heater_duty_cycle );
        controlEM ( &item->cooler, item->secure_out.cooler_duty_cycle );
        if ( !reg_secureNeed ( &item->secure_out ) ) {
            item->state = REG_BUSY;
        }
        break;
    case REG_DISABLE:
        offEM ( &item->heater );
        offEM ( &item->cooler );
        ton_ts_reset ( &item->tmr );
        item->state_r = REG_OFF;
        item->state_onf = REG_OFF;
        item->state = REG_OFF;
        break;
    case REG_OFF:
        break;
    default:
        item->state = REG_OFF;
        break;
    }
#ifdef MODE_DEBUG
    char *state = reg_getStateStr ( item->state );
    char *state_r = reg_getStateStr ( item->state_r );
    char *state_onf = reg_getStateStr ( item->state_onf );
    struct timespec tm1 = getTimeRestTmr ( item->change_gap, item->tmr );
    printf ( "state=%s state_onf=%s EM_state=%s goal=%.1f delta_h/c=%.1f/%.1f real=%.1f real_st=%d change_time=%ldsec\n", state, state_onf, state_r, item->goal, item->heater.delta, item->cooler.delta, SNSR_VAL, item->sensor.value.state, tm1.tv_sec );
#endif
}

void regonfhc_enable ( RegOnfHC *item ) {
    item->state = REG_INIT;
}

void regonfhc_disable ( RegOnfHC *item ) {
    item->state = REG_DISABLE;
}

int regonfhc_getEnabled ( const RegOnfHC *item ) {
    if ( item->state == REG_DISABLE || item->state == REG_OFF ) {
        return 0;
    }
    return 1;
}

void regonfhc_setCoolerDelta ( RegOnfHC *item, double value ) {
    item->cooler.delta = value;
    if ( item->state == REG_BUSY && item->state_r == REG_COOLER ) {
        item->state = REG_INIT;
    }
}

void regonfhc_setHeaterDelta ( RegOnfHC *item, double value ) {
    item->heater.delta = value;
    if ( item->state == REG_BUSY && item->state_r == REG_HEATER ) {
        item->state = REG_INIT;
    }
}

void regonfhc_setGoal ( RegOnfHC *item, double value ) {
    item->goal = value;
    /*
        if (item->state == REG_BUSY) {
            item->state = REG_INIT;
        }
     */
}

void regonfhc_setChangeGap ( RegOnfHC *item, int value ) {
    item->change_gap.tv_sec = value;
    item->change_gap.tv_nsec = 0;
}

void regonfhc_setEMMode ( RegOnfHC *item, const char * value ) {
    if ( strcmp ( REG_EM_MODE_COOLER_STR, value ) == 0 ) {
        offEM ( &item->heater );
        item->cooler.use = 1;
        item->heater.use = 0;
    } else if ( strcmp ( REG_EM_MODE_HEATER_STR, value ) == 0 ) {
        offEM ( &item->cooler );
        item->cooler.use = 0;
        item->heater.use = 1;
    } else if ( strcmp ( REG_EM_MODE_BOTH_STR, value ) == 0 ) {
        item->cooler.use = 1;
        item->heater.use = 1;
    } else {
        offEM ( &item->heater );
        offEM ( &item->cooler );
        item->cooler.use = 0;
        item->heater.use = 0;
    }
    if ( item->state == REG_BUSY ) {
        item->state = REG_INIT;
    }
}

void regonfhc_setHeaterPower ( RegOnfHC *item, double value ) {
    controlEM ( &item->heater, value );
}

void regonfhc_setCoolerPower ( RegOnfHC *item, double value ) {
    controlEM ( &item->cooler, value );
}

void regonfhc_turnOff ( RegOnfHC *item ) {
    item->state = REG_OFF;
    offEM ( &item->cooler );
    offEM ( &item->heater );
}

void regonfhc_secureOutTouch ( RegOnfHC *item ) {
    reg_secureTouch ( &item->secure_out );
}
