/*
 * Supervisor Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Master control for system state, emergency stops, and coordination.
 */

#include "robot.h"

/*==========================================================================*/
/* Supervisor Active Object */
/*==========================================================================*/

typedef enum {
    ROBOT_STATE_INIT,
    ROBOT_STATE_IDLE,
    ROBOT_STATE_MANUAL,
    ROBOT_STATE_AUTONOMOUS,
    ROBOT_STATE_EMERGENCY,
    ROBOT_STATE_FAULT
} RobotState;

typedef struct {
    QActive super;
    
    /* System state */
    RobotState robot_state;
    
    /* Health monitoring */
    uint16_t battery_voltage;
    uint8_t battery_percent;
    bool estop_active;
    bool android_connected;
    
    /* Error flags */
    uint32_t error_flags;
    
    /* Statistics */
    uint32_t uptime_sec;
    uint32_t waypoints_reached;
    
    /* Time event for periodic heartbeat */
    QTimeEvt timeEvt;
} Supervisor;

/* Error flag bits */
#define ERR_MOTOR_FAULT     (1U << 0)
#define ERR_CANFD_ERROR     (1U << 1)
#define ERR_GPS_LOST        (1U << 2)
#define ERR_IMU_FAULT       (1U << 3)
#define ERR_VISION_FAULT    (1U << 4)
#define ERR_LOW_BATTERY     (1U << 5)
#define ERR_ANDROID_LOST    (1U << 6)

/* Battery thresholds */
#define BATTERY_LOW_MV      22000   /* 22V */
#define BATTERY_CRITICAL_MV 20000   /* 20V */
#define BATTERY_FULL_MV     25200   /* 25.2V (6S LiPo) */

/* Local instance */
static Supervisor l_supervisor;

/* Global pointer */
QActive * const AO_Supervisor = &l_supervisor.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState Supervisor_initial(Supervisor * const me, QEvt const * const e);
static QState Supervisor_idle(Supervisor * const me, QEvt const * const e);
static QState Supervisor_manual(Supervisor * const me, QEvt const * const e);
static QState Supervisor_autonomous(Supervisor * const me, QEvt const * const e);
static QState Supervisor_emergency(Supervisor * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static void Supervisor_checkBattery(Supervisor * const me) {
    me->battery_voltage = BSP_adc_read(0);  /* Read battery ADC */
    
    /* Calculate percentage (assuming 6S LiPo: 20V-25.2V range) */
    if (me->battery_voltage >= BATTERY_FULL_MV) {
        me->battery_percent = 100;
    } else if (me->battery_voltage <= BATTERY_CRITICAL_MV) {
        me->battery_percent = 0;
    } else {
        me->battery_percent = (uint8_t)(
            (me->battery_voltage - BATTERY_CRITICAL_MV) * 100 / 
            (BATTERY_FULL_MV - BATTERY_CRITICAL_MV));
    }
    
    /* Set error flags */
    if (me->battery_voltage < BATTERY_LOW_MV) {
        me->error_flags |= ERR_LOW_BATTERY;
    } else {
        me->error_flags &= ~ERR_LOW_BATTERY;
    }
}

static void Supervisor_checkEstop(Supervisor * const me) {
    me->estop_active = BSP_is_estop_active();
}

static void Supervisor_broadcastHeartbeat(Supervisor * const me) {
    (void)me;
    
    /* Send heartbeat to all active objects */
    QEvt *evt = Q_NEW(QEvt, SIG_HEARTBEAT);
    if (evt) {
        QActive_post(AO_MotorCtrl, evt, 0U);
    }
    
    evt = Q_NEW(QEvt, SIG_HEARTBEAT);
    if (evt) {
        QActive_post(AO_TurretCtrl, evt, 0U);
    }
}

static void Supervisor_broadcastEmergencyStop(Supervisor * const me) {
    (void)me;
    
    QEvt *evt;
    
    /* Stop all motors */
    evt = Q_NEW(QEvt, SIG_EMERGENCY_STOP);
    if (evt) QActive_post(AO_MotorCtrl, evt, 0U);
    
    /* Stop turret */
    evt = Q_NEW(QEvt, SIG_EMERGENCY_STOP);
    if (evt) QActive_post(AO_TurretCtrl, evt, 0U);
    
    /* Stop path planner */
    evt = Q_NEW(QEvt, SIG_EMERGENCY_STOP);
    if (evt) QActive_post(AO_PathPlanner, evt, 0U);
}

static void Supervisor_updateLEDs(Supervisor * const me) {
    /* LED status indicators */
    switch (me->robot_state) {
        case ROBOT_STATE_IDLE:
            BSP_led_on(BSP_LED_GREEN_PIN);
            BSP_led_off(BSP_LED_RED_PIN);
            BSP_led_off(BSP_LED_BLUE_PIN);
            break;
        case ROBOT_STATE_MANUAL:
            BSP_led_off(BSP_LED_GREEN_PIN);
            BSP_led_off(BSP_LED_RED_PIN);
            BSP_led_on(BSP_LED_BLUE_PIN);
            break;
        case ROBOT_STATE_AUTONOMOUS:
            BSP_led_on(BSP_LED_GREEN_PIN);
            BSP_led_off(BSP_LED_RED_PIN);
            BSP_led_on(BSP_LED_BLUE_PIN);
            break;
        case ROBOT_STATE_EMERGENCY:
            BSP_led_off(BSP_LED_GREEN_PIN);
            BSP_led_on(BSP_LED_RED_PIN);
            BSP_led_off(BSP_LED_BLUE_PIN);
            break;
        default:
            BSP_led_toggle(BSP_LED_RED_PIN);
            break;
    }
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState Supervisor_initial(Supervisor * const me, QEvt const * const e) {
    (void)e;
    
    me->robot_state = ROBOT_STATE_INIT;
    me->battery_voltage = 0;
    me->battery_percent = 0;
    me->estop_active = false;
    me->android_connected = false;
    me->error_flags = 0;
    me->uptime_sec = 0;
    me->waypoints_reached = 0;
    
    /* Initialize CAN-FD for motor communication */
    BSP_canfd_init(0, BSP_CANFD_BITRATE);
    
    /* Initialize PWM for turret servos */
    BSP_pwm_init();
    
    /* Arm periodic timer (1 second heartbeat) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC, BSP_TICKS_PER_SEC);
    
    return Q_TRAN(&Supervisor_idle);
}

static QState Supervisor_idle(Supervisor * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("Supervisor: IDLE\r\n");
            me->robot_state = ROBOT_STATE_IDLE;
            Supervisor_updateLEDs(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->uptime_sec++;
            Supervisor_checkBattery(me);
            Supervisor_checkEstop(me);
            Supervisor_broadcastHeartbeat(me);
            Supervisor_updateLEDs(me);
            
            if (me->estop_active) {
                status = Q_TRAN(&Supervisor_emergency);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_ANDROID_CMD: {
            AndroidCmdEvt const *evt = Q_EVT_CAST(AndroidCmdEvt);
            switch (evt->cmd_type) {
                case 1:  /* Manual mode */
                    status = Q_TRAN(&Supervisor_manual);
                    break;
                case 2:  /* Autonomous mode */
                    status = Q_TRAN(&Supervisor_autonomous);
                    break;
                default:
                    status = Q_HANDLED();
                    break;
            }
            break;
        }
            
        case SIG_WAYPOINT_REACHED:
            me->waypoints_reached++;
            status = Q_HANDLED();
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState Supervisor_manual(Supervisor * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("Supervisor: MANUAL\r\n");
            me->robot_state = ROBOT_STATE_MANUAL;
            Supervisor_updateLEDs(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->uptime_sec++;
            Supervisor_checkBattery(me);
            Supervisor_checkEstop(me);
            
            if (me->estop_active) {
                status = Q_TRAN(&Supervisor_emergency);
            } else if (me->battery_voltage < BATTERY_CRITICAL_MV) {
                status = Q_TRAN(&Supervisor_emergency);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_ANDROID_CMD: {
            AndroidCmdEvt const *evt = Q_EVT_CAST(AndroidCmdEvt);
            if (evt->cmd_type == 0) {  /* Stop/idle */
                status = Q_TRAN(&Supervisor_idle);
            } else if (evt->cmd_type == 2) {  /* Autonomous */
                status = Q_TRAN(&Supervisor_autonomous);
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState Supervisor_autonomous(Supervisor * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("Supervisor: AUTONOMOUS\r\n");
            me->robot_state = ROBOT_STATE_AUTONOMOUS;
            Supervisor_updateLEDs(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->uptime_sec++;
            Supervisor_checkBattery(me);
            Supervisor_checkEstop(me);
            
            if (me->estop_active) {
                status = Q_TRAN(&Supervisor_emergency);
            } else if (me->battery_voltage < BATTERY_CRITICAL_MV) {
                Supervisor_broadcastEmergencyStop(me);
                status = Q_TRAN(&Supervisor_emergency);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_WAYPOINT_REACHED:
            me->waypoints_reached++;
            status = Q_HANDLED();
            break;
            
        case SIG_ANDROID_CMD: {
            AndroidCmdEvt const *evt = Q_EVT_CAST(AndroidCmdEvt);
            if (evt->cmd_type == 0) {  /* Stop */
                Supervisor_broadcastEmergencyStop(me);
                status = Q_TRAN(&Supervisor_idle);
            } else if (evt->cmd_type == 1) {  /* Manual */
                status = Q_TRAN(&Supervisor_manual);
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_OBSTACLE_DETECTED:
            /* Forward to path planner */
            QActive_post(AO_PathPlanner, e, 0U);
            status = Q_HANDLED();
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState Supervisor_emergency(Supervisor * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("Supervisor: EMERGENCY\r\n");
            me->robot_state = ROBOT_STATE_EMERGENCY;
            Supervisor_broadcastEmergencyStop(me);
            Supervisor_updateLEDs(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->uptime_sec++;
            Supervisor_checkBattery(me);
            Supervisor_checkEstop(me);
            Supervisor_updateLEDs(me);
            
            /* Check if emergency condition cleared */
            if (!me->estop_active && me->battery_voltage >= BATTERY_LOW_MV) {
                status = Q_TRAN(&Supervisor_idle);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_HEARTBEAT:
            /* Acknowledge heartbeat but stay in emergency */
            status = Q_HANDLED();
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

void Supervisor_ctor(void) {
    Supervisor *me = &l_supervisor;
    
    QActive_ctor(&me->super, (QStateHandler)&Supervisor_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
