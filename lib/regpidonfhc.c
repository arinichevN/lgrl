
#include "regpidonfhc.h"
#include "reg.c"
#include "regonfhc.h"
#include "green_light.c"

static void controlEM ( RegPIDOnfHCEM *item, double output ) {
    if ( item->use ) {
        if ( output>item->output_max ) output=item->output_max;
        if ( output<item->output_min ) output=item->output_min;
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
    }
}

static void offEM ( RegPIDOnfHCEM *item ) {
    double output=item->output_min;
    if ( item->use ) {
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
    }
}
static int checkEM ( const RegPIDOnfHCEM *item ) {
    int success=1;
    if ( item->output_min > item->output_max ) {
        fprintf ( stderr, "%s(): output_min <= output_max expected where remote_channel_id = %d\n", F, item->remote_channel.id );
        success= 0;
    }
    if ( ! ( item->mode == REG_MODE_PID ||  item->mode == REG_MODE_ONF ) ) {
        fprintf ( stderr, "%s(): bad mode where remote_channel_id = %d\n", F, item->remote_channel.id );
        success= 0;
    }
    if ( item->delta < 0.0 ) {
        fprintf ( stderr, "%s(): bad delta where remote_channel_id = %d\n", F, item->remote_channel.id );
        success= 0;
    }
    return success;
}
int regpidonfhc_check (const RegPIDOnfHC *item ) {
    int success=1;
    if ( !checkEM(&item->heater) ) {
        fprintf ( stderr, "%s(): bad heater data\n", F );
        success= 0;
    }
    if ( !checkEM(&item->cooler) ) {
        fprintf ( stderr, "%s(): bad heater data\n", F );
        success= 0;
    }
    if ( ! ( item->heater.use || item->cooler.use ) ) {
        fprintf ( stderr, "%s(): you should use heater or cooler or both\n", F );
        success= 0;
    }
    if ( item->change_gap.tv_sec < 0  ) {
        fprintf ( stderr, "%s(): change_gap_sec >= 0 expected\n", F );
        success= 0;
    }
    if (  item->change_gap.tv_nsec < 0 ) {
        fprintf ( stderr, "%s(): change_gap_nsec >= 0 expected\n", F );
        success= 0;
    }
    return success;
}
int regpidonfhc_init ( RegPIDOnfHC *item, int *fd ) {
    if ( !initRChannel ( &item->sensor.remote_channel, fd ) ) {
        printde ( "failed where remote channel id = %d\n", item->sensor.remote_channel.id );
        return 0;
    }
    if ( !initRChannel ( &item->heater.remote_channel, fd ) ) {
        printde ( "failed where remote channel id = %d\n", item->heater.remote_channel.id );
        return 0;
    }
    if ( !initRChannel ( &item->cooler.remote_channel, fd ) ) {
        printde ( "failed where remote channel id = %d\n", item->cooler.remote_channel.id );
        return 0;
    }
    if ( item->green_light.active ) {
        if ( !greenLight_init ( &item->green_light, fd ) ) {
            printde ( "failed where green light id = %d\n", item->green_light.id );
            return 0;
        }
    }
    return 1;
}
void regpidonfhc_control ( RegPIDOnfHC *item ) {
    double output;
    switch ( item->state ) {
    case REG_INIT:
        if ( !greenLight_isGreen ( &item->green_light ) ) {
            putsde ( "light is not green\n" );
            return;
        }
        if ( !reg_sensorRead ( &item->sensor ) ) {
            putsde ( "reading from sensor failed\n" );
            return;
        }
        ton_ts_reset ( &item->tmr );
        offEM ( &item->heater );
        offEM ( &item->cooler );
        item->snsrf_count = 0;
        item->heater.pid.mode = PID_MODE_HEATER;
        item->cooler.pid.mode = PID_MODE_COOLER;
        item->heater.pid.reset = 1;
        item->cooler.pid.reset = 1;
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
        if ( reg_sensorRead ( &item->sensor ) ) {
            item->snsrf_count = 0;
            int value_is_out = 0;
            char other_em;
            RegPIDOnfHCEM *reg_em = NULL;
            RegPIDOnfHCEM *reg_em_other = NULL;
            switch ( item->state_r ) {
            case REG_HEATER:
                switch ( item->heater.mode ) {
                case REG_MODE_PID:
                    if ( item->heater.pid.previous_output < 0.0 ) {
                        value_is_out = 1;
                    }
                    break;
                case REG_MODE_ONF:
                    if ( VAL_IS_OUT_H ) {
                        value_is_out = 1;
                    }
                    break;
                }
                other_em = REG_COOLER;
                reg_em = &item->heater;
                reg_em_other = &item->cooler;
                break;
            case REG_COOLER:
                switch ( item->cooler.mode ) {
                case REG_MODE_PID:
                    if ( item->cooler.pid.previous_output < 0.0 ) {
                        value_is_out = 1;
                    }
                    break;
                case REG_MODE_ONF:
                    if ( VAL_IS_OUT_C ) {
                        value_is_out = 1;
                    }
                    break;
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
            switch ( reg_em->mode ) {
            case REG_MODE_PID:
                output = pid ( &reg_em->pid, item->goal, SNSR_VAL );
                output= ( output > reg_em->output_max ) ? reg_em->output_max : output;
                output= ( output < reg_em->output_min ) ? reg_em->output_min : output;
                break;
            case REG_MODE_ONF:
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
    char *heater_mode = reg_getStateStr ( item->heater.mode );
    char *cooler_mode = reg_getStateStr ( item->cooler.mode );
    struct timespec tm1 = getTimeRestTmr ( item->change_gap, item->tmr );
    printf ( "state=%s state_onf=%s EM_state=%s mode(h/c)=%s/%s goal=%.1f real=%.1f out(h/c)=%.1f/%.1f change_time=%ldsec\n", state, state_onf, state_r, heater_mode, cooler_mode, item->goal, SNSR_VAL, item->heater.output,item->cooler.output, tm1.tv_sec );
#endif
}

void regpidonfhc_enable ( RegPIDOnfHC *item ) {
    item->state = REG_INIT;
}

void regpidonfhc_disable ( RegPIDOnfHC *item ) {
    item->state = REG_DISABLE;
}

int regpidonfhc_getEnabled ( const RegPIDOnfHC *item ) {
    if ( item->state == REG_DISABLE || item->state == REG_OFF ) {
        return 0;
    }
    return 1;
}

void regpidonfhc_setCoolerDelta ( RegPIDOnfHC *item, double value ) {
    item->cooler.delta = value;
    if ( item->state == REG_BUSY && item->heater.mode == REG_MODE_ONF && item->state_r == REG_COOLER ) {
        item->state = REG_INIT;
    }
}

void regpidonfhc_setHeaterDelta ( RegPIDOnfHC *item, double value ) {
    item->heater.delta = value;
    if ( item->state == REG_BUSY && item->heater.mode == REG_MODE_ONF && item->state_r == REG_HEATER ) {
        item->state = REG_INIT;
    }
}

void regpidonfhc_setHeaterKp ( RegPIDOnfHC *item, double value ) {
    item->heater.pid.kp = value;
}

void regpidonfhc_setHeaterKi ( RegPIDOnfHC *item, double value ) {
    item->heater.pid.ki = value;
}

void regpidonfhc_setHeaterKd ( RegPIDOnfHC *item, double value ) {
    item->heater.pid.kd = value;
}

void regpidonfhc_setCoolerKp ( RegPIDOnfHC *item, double value ) {
    item->cooler.pid.kp = value;
}

void regpidonfhc_setCoolerKi ( RegPIDOnfHC *item, double value ) {
    item->cooler.pid.ki = value;
}

void regpidonfhc_setCoolerKd ( RegPIDOnfHC *item, double value ) {
    item->cooler.pid.kd = value;
}

void regpidonfhc_setGoal ( RegPIDOnfHC *item, double value ) {
    item->goal = value;
    /*
        if (item->state == REG_BUSY) {
            item->state = REG_INIT;
        }
     */
}

void regpidonfhc_setHeaterMode ( RegPIDOnfHC *item, const char * value ) {
    if ( strncmp ( value, REG_MODE_PID_STR, 3 ) == 0 ) {
        item->heater.mode = REG_MODE_PID;
    } else if ( strncmp ( value, REG_MODE_ONF_STR, 3 ) == 0 ) {
        item->heater.mode = REG_MODE_ONF;
    } else {
        return;
    }
    if ( item->state == REG_BUSY && item->state_r == REG_HEATER ) {
        item->state = REG_INIT;
    }
}

void regpidonfhc_setCoolerMode ( RegPIDOnfHC *item, const char * value ) {
    if ( strncmp ( value, REG_MODE_PID_STR, 3 ) == 0 ) {
        item->cooler.mode = REG_MODE_PID;
    } else if ( strncmp ( value, REG_MODE_ONF_STR, 3 ) == 0 ) {
        item->cooler.mode = REG_MODE_ONF;
    } else {
        return;
    }
    if ( item->state == REG_BUSY && item->state_r == REG_COOLER ) {
        item->state = REG_INIT;
    }
}

void regpidonfhc_setEMMode ( RegPIDOnfHC *item, const char * value ) {
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

void regpidonfhc_setChangeGap ( RegPIDOnfHC *item, int value ) {
    item->change_gap.tv_sec = value;
    item->change_gap.tv_nsec = 0;
}

void regpidonfhc_setHeaterPower ( RegPIDOnfHC *item, double value ) {
    controlEM ( &item->heater, value );
}

void regpidonfhc_setCoolerPower ( RegPIDOnfHC *item, double value ) {
    controlEM ( &item->cooler, value );
}

void regpidonfhc_turnOff ( RegPIDOnfHC *item ) {
    item->state = REG_OFF;
    offEM ( &item->cooler );
    offEM ( &item->heater );
}

void regpidonfhc_secureOutTouch ( RegPIDOnfHC *item ) {
    reg_secureTouch ( &item->secure_out );
}
