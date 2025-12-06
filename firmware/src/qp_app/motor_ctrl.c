/*
 * Motor Control Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Controls all four wheel motors via CAN-FD bus.
 */

#include "robot.h"

/*==========================================================================*/
/* Motor Control Active Object */
/*==========================================================================*/

typedef struct {
    QActive super;
    
    /* Motor states */
    int16_t motor_speed[ROBOT_NUM_WHEELS];
    int32_t motor_position[ROBOT_NUM_WHEELS];
    uint8_t motor_status[ROBOT_NUM_WHEELS];
    
    /* Control parameters */
    bool enabled;
    bool emergency_stop;
    
    /* Time event for periodic status update */
    QTimeEvt timeEvt;
} MotorCtrl;

/* Local instance */
static MotorCtrl l_motorCtrl;

/* Global pointer */
QActive * const AO_MotorCtrl = &l_motorCtrl.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState MotorCtrl_initial(MotorCtrl * const me, QEvt const * const e);
static QState MotorCtrl_idle(MotorCtrl * const me, QEvt const * const e);
static QState MotorCtrl_running(MotorCtrl * const me, QEvt const * const e);
static QState MotorCtrl_stopped(MotorCtrl * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static void MotorCtrl_sendCANFD(MotorCtrl * const me, uint8_t motor_id, 
                                 int16_t speed) {
    uint8_t data[8];
    uint32_t can_id;
    
    /* Determine CAN ID based on motor */
    switch (motor_id) {
        case 0: can_id = CANFD_NODE_MOTOR_FL; break;
        case 1: can_id = CANFD_NODE_MOTOR_FR; break;
        case 2: can_id = CANFD_NODE_MOTOR_RL; break;
        case 3: can_id = CANFD_NODE_MOTOR_RR; break;
        default: return;
    }
    
    /* Build CAN-FD message */
    data[0] = 0x01;  /* Command: set speed */
    data[1] = (uint8_t)(speed & 0xFF);
    data[2] = (uint8_t)((speed >> 8) & 0xFF);
    data[3] = me->enabled ? 0x01 : 0x00;
    data[4] = 0x00;
    data[5] = 0x00;
    data[6] = 0x00;
    data[7] = 0x00;
    
    BSP_canfd_send(0, can_id, data, 8);
}

static void MotorCtrl_stopAll(MotorCtrl * const me) {
    for (uint8_t i = 0; i < ROBOT_NUM_WHEELS; i++) {
        me->motor_speed[i] = 0;
        MotorCtrl_sendCANFD(me, i, 0);
    }
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState MotorCtrl_initial(MotorCtrl * const me, QEvt const * const e) {
    (void)e;
    
    /* Initialize motor states */
    for (uint8_t i = 0; i < ROBOT_NUM_WHEELS; i++) {
        me->motor_speed[i] = 0;
        me->motor_position[i] = 0;
        me->motor_status[i] = 0;
    }
    
    me->enabled = false;
    me->emergency_stop = false;
    
    /* Arm periodic timer for status updates (100ms) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC / 10, BSP_TICKS_PER_SEC / 10);
    
    return Q_TRAN(&MotorCtrl_idle);
}

static QState MotorCtrl_idle(MotorCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("MotorCtrl: IDLE\r\n");
            me->enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED: {
            MotorEvt const *evt = Q_EVT_CAST(MotorEvt);
            if (evt->motor_id < ROBOT_NUM_WHEELS) {
                me->motor_speed[evt->motor_id] = evt->speed;
            }
            me->enabled = true;
            status = Q_TRAN(&MotorCtrl_running);
            break;
        }
        
        case SIG_TIMEOUT:
            /* Periodic status check */
            status = Q_HANDLED();
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState MotorCtrl_running(MotorCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("MotorCtrl: RUNNING\r\n");
            me->enabled = true;
            /* Send current speeds to all motors */
            for (uint8_t i = 0; i < ROBOT_NUM_WHEELS; i++) {
                MotorCtrl_sendCANFD(me, i, me->motor_speed[i]);
            }
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED: {
            MotorEvt const *evt = Q_EVT_CAST(MotorEvt);
            if (evt->motor_id < ROBOT_NUM_WHEELS) {
                me->motor_speed[evt->motor_id] = evt->speed;
                MotorCtrl_sendCANFD(me, evt->motor_id, evt->speed);
            }
            status = Q_HANDLED();
            break;
        }
        
        case SIG_MOTOR_STOP:
            status = Q_TRAN(&MotorCtrl_stopped);
            break;
            
        case SIG_EMERGENCY_STOP:
            me->emergency_stop = true;
            status = Q_TRAN(&MotorCtrl_stopped);
            break;
            
        case SIG_TIMEOUT:
            /* Periodic speed update to motors */
            for (uint8_t i = 0; i < ROBOT_NUM_WHEELS; i++) {
                MotorCtrl_sendCANFD(me, i, me->motor_speed[i]);
            }
            status = Q_HANDLED();
            break;
            
        case SIG_CANFD_RX: {
            /* Handle motor feedback */
            CANFDEvt const *evt = Q_EVT_CAST(CANFDEvt);
            uint8_t motor_id = 0xFF;
            
            /* Determine which motor sent the message */
            if (evt->id >= 0x110 && evt->id <= 0x113) {
                motor_id = evt->id - 0x110;
            }
            
            if (motor_id < ROBOT_NUM_WHEELS) {
                /* Parse feedback data */
                me->motor_status[motor_id] = evt->data[0];
                me->motor_position[motor_id] = 
                    (int32_t)evt->data[1] |
                    ((int32_t)evt->data[2] << 8) |
                    ((int32_t)evt->data[3] << 16) |
                    ((int32_t)evt->data[4] << 24);
            }
            status = Q_HANDLED();
            break;
        }
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState MotorCtrl_stopped(MotorCtrl * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("MotorCtrl: STOPPED\r\n");
            MotorCtrl_stopAll(me);
            me->enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED:
            if (!me->emergency_stop) {
                status = Q_TRAN(&MotorCtrl_running);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_HEARTBEAT:
            /* Clear emergency stop on heartbeat */
            me->emergency_stop = false;
            status = Q_TRAN(&MotorCtrl_idle);
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

void MotorCtrl_ctor(void) {
    MotorCtrl *me = &l_motorCtrl;
    
    QActive_ctor(&me->super, (QStateHandler)&MotorCtrl_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
