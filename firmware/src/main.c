/*
 * Robot Main Entry Point
 * FRDM-MCXN947 4WD Robot with QP Framework
 */

#include "robot.h"

/*==========================================================================*/
/* Event Queues for Active Objects */
/*==========================================================================*/

static QEvt const *motorCtrlQueueSto[16];
static QEvt const *turretCtrlQueueSto[16];
static QEvt const *pathPlannerQueueSto[32];
static QEvt const *sensorFusionQueueSto[32];
static QEvt const *androidCommQueueSto[64];
static QEvt const *supervisorQueueSto[16];

/*==========================================================================*/
/* Event Pool */
/*==========================================================================*/

static union {
    void *min_size;
    uint8_t storage[4096];
} eventPoolSto;

/*==========================================================================*/
/* Main Function */
/*==========================================================================*/

int main(void) {
    /* Initialize Board Support Package */
    BSP_init();
    
    /* Initialize QP framework */
    QF_init();
    
    /* Initialize event pool */
    QF_poolInit(eventPoolSto.storage, sizeof(eventPoolSto), 
                sizeof(CANFDEvt));  /* Largest event */
    
    /* Construct all Active Objects */
    MotorCtrl_ctor();
    TurretCtrl_ctor();
    PathPlanner_ctor();
    SensorFusion_ctor();
    AndroidComm_ctor();
    Supervisor_ctor();
    
    /* Start Active Objects */
    QActive_start(AO_MotorCtrl, AO_PRIO_MOTOR_CTRL,
                  motorCtrlQueueSto, sizeof(motorCtrlQueueSto)/sizeof(motorCtrlQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    QActive_start(AO_TurretCtrl, AO_PRIO_TURRET_CTRL,
                  turretCtrlQueueSto, sizeof(turretCtrlQueueSto)/sizeof(turretCtrlQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    QActive_start(AO_PathPlanner, AO_PRIO_PATH_PLANNER,
                  pathPlannerQueueSto, sizeof(pathPlannerQueueSto)/sizeof(pathPlannerQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    QActive_start(AO_SensorFusion, AO_PRIO_SENSOR_FUSION,
                  sensorFusionQueueSto, sizeof(sensorFusionQueueSto)/sizeof(sensorFusionQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    QActive_start(AO_AndroidComm, AO_PRIO_ANDROID_COMM,
                  androidCommQueueSto, sizeof(androidCommQueueSto)/sizeof(androidCommQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    QActive_start(AO_Supervisor, AO_PRIO_SUPERVISOR,
                  supervisorQueueSto, sizeof(supervisorQueueSto)/sizeof(supervisorQueueSto[0]),
                  (void *)0, 0U, (void *)0);
    
    /* Start BSP (enables interrupts) */
    BSP_start();
    
    /* Run QP framework (never returns) */
    return QF_run();
}
