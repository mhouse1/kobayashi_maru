/*
 * Turret Control Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Controls pan/tilt turret via PWM servos.
 */

#include "robot.h"

/*==========================================================================*/
/* Turret Control Active Object */
/*==========================================================================*/

typedef struct {
    QActive super;
    
    /* Turret position state */
    int16_t pan_setpoint;     /* Degrees * 100 */
    int16_t tilt_setpoint;    /* Degrees * 100 */
    int16_t pan_actual;       /* Degrees * 100 */
    int16_t tilt_actual;      /* Degrees * 100 */
    
    /* Turret speed */
    uint16_t pan_speed;       /* Degrees/sec */
    uint16_t tilt_speed;      /* Degrees/sec */
    
    /* Control flags */
    bool enabled;
    bool tracking_mode;
    bool homed;
    
    /* Vision target for tracking */
    uint16_t target_x;
    uint16_t target_y;
    
    /* Time event for periodic update */
    QTimeEvt timeEvt;
} TurretCtrl;

/* Turret limits */
#define TURRET_PAN_MIN      (-18000)  /* -180 degrees */
#define TURRET_PAN_MAX      (18000)   /* +180 degrees */
#define TURRET_TILT_MIN     (-4500)   /* -45 degrees */
#define TURRET_TILT_MAX     (9000)    /* +90 degrees */

/* PWM channels */
#define PWM_CHANNEL_PAN     0
#define PWM_CHANNEL_TILT    1

/* Local instance */
static TurretCtrl l_turretCtrl;

/* Global pointer */
QActive * const AO_TurretCtrl = &l_turretCtrl.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState TurretCtrl_initial(TurretCtrl * const me, QEvt const * const e);
static QState TurretCtrl_idle(TurretCtrl * const me, QEvt const * const e);
static QState TurretCtrl_homing(TurretCtrl * const me, QEvt const * const e);
static QState TurretCtrl_positioning(TurretCtrl * const me, QEvt const * const e);
static QState TurretCtrl_tracking(TurretCtrl * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static int16_t clamp_pan(int16_t value) {
    if (value < TURRET_PAN_MIN) return TURRET_PAN_MIN;
    if (value > TURRET_PAN_MAX) return TURRET_PAN_MAX;
    return value;
}

static int16_t clamp_tilt(int16_t value) {
    if (value < TURRET_TILT_MIN) return TURRET_TILT_MIN;
    if (value > TURRET_TILT_MAX) return TURRET_TILT_MAX;
    return value;
}

static uint16_t angle_to_pwm(int16_t angle, int16_t min_angle, int16_t max_angle) {
    /* Convert angle (degrees * 100) to PWM duty cycle (1000-2000us) */
    int32_t range = max_angle - min_angle;
    int32_t normalized = angle - min_angle;
    return (uint16_t)(1000 + (normalized * 1000) / range);
}

static void TurretCtrl_updatePWM(TurretCtrl * const me) {
    uint16_t pan_pwm = angle_to_pwm(me->pan_actual, TURRET_PAN_MIN, TURRET_PAN_MAX);
    uint16_t tilt_pwm = angle_to_pwm(me->tilt_actual, TURRET_TILT_MIN, TURRET_TILT_MAX);
    
    BSP_pwm_set_duty(PWM_CHANNEL_PAN, pan_pwm);
    BSP_pwm_set_duty(PWM_CHANNEL_TILT, tilt_pwm);
}

static void TurretCtrl_moveToSetpoint(TurretCtrl * const me) {
    /* Move pan towards setpoint */
    int16_t pan_step = me->pan_speed / 100;  /* Step per 10ms update */
    if (me->pan_actual < me->pan_setpoint) {
        me->pan_actual += pan_step;
        if (me->pan_actual > me->pan_setpoint) {
            me->pan_actual = me->pan_setpoint;
        }
    } else if (me->pan_actual > me->pan_setpoint) {
        me->pan_actual -= pan_step;
        if (me->pan_actual < me->pan_setpoint) {
            me->pan_actual = me->pan_setpoint;
        }
    }
    
    /* Move tilt towards setpoint */
    int16_t tilt_step = me->tilt_speed / 100;
    if (me->tilt_actual < me->tilt_setpoint) {
        me->tilt_actual += tilt_step;
        if (me->tilt_actual > me->tilt_setpoint) {
            me->tilt_actual = me->tilt_setpoint;
        }
    } else if (me->tilt_actual > me->tilt_setpoint) {
        me->tilt_actual -= tilt_step;
        if (me->tilt_actual < me->tilt_setpoint) {
            me->tilt_actual = me->tilt_setpoint;
        }
    }
    
    TurretCtrl_updatePWM(me);
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState TurretCtrl_initial(TurretCtrl * const me, QEvt const * const e) {
    (void)e;
    
    me->pan_setpoint = 0;
    me->tilt_setpoint = 0;
    me->pan_actual = 0;
    me->tilt_actual = 0;
    me->pan_speed = 3000;   /* 30 deg/sec */
    me->tilt_speed = 2000;  /* 20 deg/sec */
    me->enabled = false;
    me->tracking_mode = false;
    me->homed = false;
    me->target_x = 0;
    me->target_y = 0;
    
    /* Arm periodic timer (10ms) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC / 100, BSP_TICKS_PER_SEC / 100);
    
    return Q_TRAN(&TurretCtrl_idle);
}

static QState TurretCtrl_idle(TurretCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("TurretCtrl: IDLE\r\n");
            me->enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = Q_TRAN(&TurretCtrl_homing);
            break;
            
        case SIG_TURRET_SET_PAN:
        case SIG_TURRET_SET_TILT: {
            TurretEvt const *evt = Q_EVT_CAST(TurretEvt);
            me->pan_setpoint = clamp_pan(evt->pan_angle);
            me->tilt_setpoint = clamp_tilt(evt->tilt_angle);
            if (evt->speed > 0) {
                me->pan_speed = evt->speed;
                me->tilt_speed = evt->speed;
            }
            status = Q_TRAN(&TurretCtrl_positioning);
            break;
        }
            
        case SIG_TURRET_TRACK:
            status = Q_TRAN(&TurretCtrl_tracking);
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState TurretCtrl_homing(TurretCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("TurretCtrl: HOMING\r\n");
            me->enabled = true;
            me->pan_setpoint = 0;
            me->tilt_setpoint = 0;
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            TurretCtrl_moveToSetpoint(me);
            if (me->pan_actual == 0 && me->tilt_actual == 0) {
                me->homed = true;
                status = Q_TRAN(&TurretCtrl_idle);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_EMERGENCY_STOP:
            status = Q_TRAN(&TurretCtrl_idle);
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState TurretCtrl_positioning(TurretCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("TurretCtrl: POSITIONING\r\n");
            me->enabled = true;
            me->tracking_mode = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_SET_PAN:
        case SIG_TURRET_SET_TILT: {
            TurretEvt const *evt = Q_EVT_CAST(TurretEvt);
            me->pan_setpoint = clamp_pan(evt->pan_angle);
            me->tilt_setpoint = clamp_tilt(evt->tilt_angle);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            TurretCtrl_moveToSetpoint(me);
            /* Stay in positioning until new command or stop */
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = Q_TRAN(&TurretCtrl_homing);
            break;
            
        case SIG_TURRET_TRACK:
            status = Q_TRAN(&TurretCtrl_tracking);
            break;
            
        case SIG_EMERGENCY_STOP:
            status = Q_TRAN(&TurretCtrl_idle);
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState TurretCtrl_tracking(TurretCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("TurretCtrl: TRACKING\r\n");
            me->enabled = true;
            me->tracking_mode = true;
            status = Q_HANDLED();
            break;
            
        case SIG_VISION_TARGET: {
            VisionEvt const *evt = Q_EVT_CAST(VisionEvt);
            me->target_x = evt->target_x;
            me->target_y = evt->target_y;
            
            /* Convert pixel coordinates to pan/tilt angles */
            /* Assume 640x480 resolution, center is (320, 240) */
            int16_t pan_error = (int16_t)(320 - evt->target_x);
            int16_t tilt_error = (int16_t)(240 - evt->target_y);
            
            /* Simple proportional control */
            me->pan_setpoint = clamp_pan(me->pan_actual + pan_error * 10);
            me->tilt_setpoint = clamp_tilt(me->tilt_actual + tilt_error * 10);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            TurretCtrl_moveToSetpoint(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = Q_TRAN(&TurretCtrl_homing);
            break;
            
        case SIG_EMERGENCY_STOP:
            status = Q_TRAN(&TurretCtrl_idle);
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

/*==========================================================================*/
/* Constructor */
/*==========================================================================*/

/* Placeholder for QHsm_top */
static QState QHsm_top(void * const me, QEvt const * const e) {
    (void)me;
    (void)e;
    return Q_IGNORED();
}

void TurretCtrl_ctor(void) {
    TurretCtrl *me = &l_turretCtrl;
    
    QActive_ctor(&me->super, (QStateHandler)&TurretCtrl_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
