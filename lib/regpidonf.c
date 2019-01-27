
#include "regpidonf.h"
#include "reg.c"

static void controlEM ( RegPIDOnfEM *item, double output ) {
    if ( output>item->output_max ) output=item->output_max;
    if ( output<item->output_min ) output=item->output_min;
    reg_controlRChannel ( &item->remote_channel, output );
    item->output = output;
}

static void offEM ( RegPIDOnfEM *item ) {
    double output=item->output_min;
        reg_controlRChannel ( &item->remote_channel, output );
        item->output = output;
}

void regpidonf_control ( RegPIDOnf *item ) {
     double output;
    switch ( item->state ) {
    case REG_INIT:
        offEM ( &item->em );
        item->snsrf_count = 0;
        item->pid.reset = 1;
        item->state_onf = REG_WAIT;
        item->state = REG_BUSY;
        break;
    case REG_BUSY: {
        if ( !reg_sensorRead(&item->sensor) ) {
            item->snsrf_count = 0;
            switch ( item->mode ) {
            case REG_MODE_PID:
                output = pid ( &item->pid, item->goal, SNSR_VAL );
                break;
            case REG_MODE_ONF:
                switch ( item->state_onf ) {
                case REG_DO:
                    switch ( item->state_r ) {
                    case REG_HEATER:
                        if ( SNSR_VAL > item->goal + item->delta ) {
                            item->state_onf = REG_WAIT;
                        }
                        break;
                    case REG_COOLER:
                        if ( SNSR_VAL < item->goal - item->delta ) {
                            item->state_onf = REG_WAIT;
                        }
                        break;
                    }
                    output = item->em.output_max;
                    break;
                case REG_WAIT:
                    switch ( item->state_r ) {
                    case REG_HEATER:
                        if ( SNSR_VAL < item->goal - item->delta ) {
                            item->state_onf = REG_DO;
                        }
                        break;
                    case REG_COOLER:
                        if ( SNSR_VAL > item->goal + item->delta ) {
                            item->state_onf = REG_DO;
                        }
                        break;
                    }
                    output =item->em.output_min;
                    break;
                }
                break;
            }
            controlEM ( &item->em, output );
        } else {
            if ( item->snsrf_count > SNSRF_COUNT_MAX ) {
                offEM ( &item->em );
                item->state = REG_INIT;
                putsde ( "reading from sensor failed, EM turned off\n" );
            } else {
                item->snsrf_count++;
                printde ( "sensor failure counter: %d\n", item->snsrf_count );
            }
        }
        break;
    }
    case REG_DISABLE:
        offEM ( &item->em );
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
    printf ( "state=%s state_onf=%s EM_state=%s goal=%.1f real=%.1f out=%.1f\n", state, state_onf, state_r, item->goal, SNSR_VAL, output );
#endif
}

void regpidonf_enable ( RegPIDOnf *item ) {
    item->state = REG_INIT;
}

void regpidonf_disable ( RegPIDOnf *item ) {
    item->state = REG_DISABLE;
}

int regpidonf_getEnabled ( const RegPIDOnf *item ) {
    if ( item->state==REG_DISABLE || item->state==REG_OFF ) {
        return 0;
    }
    return 1;
}
void regpidonf_setDelta ( RegPIDOnf *item, double value ) {
    item->delta = value;
    if ( item->state == REG_BUSY && item->mode == REG_MODE_ONF && item->state_r == REG_HEATER ) {
        item->state = REG_INIT;
    }
}

void regpidonf_setKp ( RegPIDOnf *item, double value ) {
    item->pid.kp = value;
}

void regpidonf_setKi ( RegPIDOnf *item, double value ) {
    item->pid.ki = value;
}

void regpidonf_setKd ( RegPIDOnf *item, double value ) {
    item->pid.kd = value;
}

void regpidonf_setGoal ( RegPIDOnf *item, double value ) {
    item->goal = value;
    /*
        if (item->state == REG_BUSY) {
            item->state = REG_INIT;
        }
    */
}

void regpidonf_setMode ( RegPIDOnf *item, const char * value ) {
    if ( strncmp ( value, REG_MODE_PID_STR, 3 ) == 0 ) {
        item->mode = REG_MODE_PID;
    } else if ( strncmp ( value, REG_MODE_ONF_STR, 3 ) == 0 ) {
        item->mode = REG_MODE_ONF;
    } else {
        return;
    }
    if ( item->state == REG_BUSY && item->state_r == REG_HEATER ) {
        item->state = REG_INIT;
    }
}

void regpidonf_setEMMode ( RegPIDOnf *item, const char * value ) {
    if ( strcmp ( REG_EM_MODE_COOLER_STR, value ) == 0 ) {
        item->pid.mode = PID_MODE_COOLER;
        item->state_r = REG_COOLER;
    } else if ( strcmp ( REG_EM_MODE_HEATER_STR, value ) == 0 ) {
        item->pid.mode = PID_MODE_HEATER;
        item->state_r = REG_HEATER;
    } else {
        return;
    }
    if ( item->state == REG_BUSY ) {
        item->state = REG_INIT;
    }
}

void regpidonf_setPower ( RegPIDOnf *item, double value ) {
    controlEM ( &item->em, value );
}

void regpidonf_turnOff ( RegPIDOnf *item ) {
    item->state = REG_OFF;
    offEM ( &item->em);
}

int regpidonf_check ( const RegPIDOnf *item ) {
    if ( item->mode != REG_MODE_PID && item->mode != REG_MODE_ONF ) {
        fputs ( "regpidonf_check: bad mode\n", stderr );
        return 0;
    }
    if ( item->pid.mode != PID_MODE_COOLER && item->pid.mode != PID_MODE_HEATER ) {
        fputs ( "regpidonf_check: bad pid mode\n", stderr );
        return 0;
    }
    return 1;
}
