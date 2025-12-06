/*
 * Robot Main Header - QP Framework Integration
 * FRDM-MCXN947 Freedom Board + Google Pixel 10 Pro Architecture
 * 
 * This is the main header for the heavy duty 4WD robot firmware
 * using the Quantum QP Real-Time Embedded Framework.
 */

#ifndef ROBOT_H
#define ROBOT_H

#include "qpc.h"
#include "bsp.h"

/*==========================================================================*/
/* Robot Configuration */
/*==========================================================================*/

/* Number of wheels for 4WD configuration */
#define ROBOT_NUM_WHEELS        4

/* CAN-FD Node IDs */
#define CANFD_NODE_MASTER       0x001
#define CANFD_NODE_MOTOR_FL     0x100
#define CANFD_NODE_MOTOR_FR     0x101
#define CANFD_NODE_MOTOR_RL     0x102
#define CANFD_NODE_MOTOR_RR     0x103
#define CANFD_NODE_TURRET       0x200
#define CANFD_NODE_GPS          0x300
#define CANFD_NODE_IMU          0x301
#define CANFD_NODE_VISION       0x400

/*==========================================================================*/
/* Active Object Priorities (QP framework) */
/*==========================================================================*/

enum RobotAOPriorities {
    AO_PRIO_IDLE = 0,
    AO_PRIO_MOTOR_CTRL,
    AO_PRIO_TURRET_CTRL,
    AO_PRIO_PATH_PLANNER,
    AO_PRIO_SENSOR_FUSION,
    AO_PRIO_ANDROID_COMM,
    AO_PRIO_SUPERVISOR,
    AO_PRIO_MAX
};

/*==========================================================================*/
/* Event Signals */
/*==========================================================================*/

enum RobotSignals {
    /* System signals */
    SIG_TIMEOUT = Q_USER_SIG,
    SIG_HEARTBEAT,
    SIG_EMERGENCY_STOP,
    
    /* Motor control signals */
    SIG_MOTOR_SET_SPEED,
    SIG_MOTOR_SET_POSITION,
    SIG_MOTOR_STOP,
    SIG_MOTOR_STATUS,
    
    /* Turret control signals */
    SIG_TURRET_SET_PAN,
    SIG_TURRET_SET_TILT,
    SIG_TURRET_HOME,
    SIG_TURRET_TRACK,
    SIG_TURRET_STATUS,
    
    /* Navigation signals */
    SIG_GPS_UPDATE,
    SIG_IMU_UPDATE,
    SIG_WAYPOINT_REACHED,
    SIG_PATH_UPDATE,
    SIG_OBSTACLE_DETECTED,
    
    /* Vision signals */
    SIG_VISION_TARGET,
    SIG_VISION_OBSTACLE,
    SIG_VISION_FRAME,
    
    /* Android/Pixel 10 Pro communication signals */
    SIG_ANDROID_CMD,
    SIG_ANDROID_GPS,
    SIG_ANDROID_ACCEL,
    SIG_ANDROID_VISION,
    SIG_ANDROID_STATUS,
    
    /* CAN-FD signals */
    SIG_CANFD_RX,
    SIG_CANFD_TX_DONE,
    SIG_CANFD_ERROR,
    
    SIG_MAX
};

/*==========================================================================*/
/* Event Structures */
/*==========================================================================*/

/* Motor control event */
typedef struct {
    QEvt super;
    uint8_t motor_id;
    int16_t speed;      /* RPM, negative for reverse */
    int32_t position;   /* Encoder ticks */
} MotorEvt;

/* Turret control event */
typedef struct {
    QEvt super;
    int16_t pan_angle;   /* Degrees * 100 */
    int16_t tilt_angle;  /* Degrees * 100 */
    uint16_t speed;      /* Degrees/sec */
} TurretEvt;

/* GPS data event (from Pixel 10 Pro) */
typedef struct {
    QEvt super;
    int32_t latitude;    /* Degrees * 1e7 */
    int32_t longitude;   /* Degrees * 1e7 */
    int32_t altitude;    /* mm above sea level */
    uint16_t heading;    /* Degrees * 100 */
    uint16_t speed;      /* mm/s */
    uint8_t satellites;
    uint8_t fix_quality;
} GPSEvt;

/* IMU/Accelerometer event (from Pixel 10 Pro) */
typedef struct {
    QEvt super;
    int16_t accel_x;     /* mg */
    int16_t accel_y;     /* mg */
    int16_t accel_z;     /* mg */
    int16_t gyro_x;      /* mdps (milli-degrees per second) */
    int16_t gyro_y;      /* mdps */
    int16_t gyro_z;      /* mdps */
    int16_t mag_x;       /* mGauss */
    int16_t mag_y;       /* mGauss */
    int16_t mag_z;       /* mGauss */
} IMUEvt;

/* Vision event (from Pixel 10 Pro) */
typedef struct {
    QEvt super;
    uint16_t target_x;      /* Pixel X coordinate */
    uint16_t target_y;      /* Pixel Y coordinate */
    uint16_t target_width;  /* Bounding box width */
    uint16_t target_height; /* Bounding box height */
    uint8_t target_class;   /* Object classification */
    uint8_t confidence;     /* Detection confidence 0-100 */
} VisionEvt;

/* Path waypoint event */
typedef struct {
    QEvt super;
    int32_t latitude;
    int32_t longitude;
    uint8_t waypoint_id;
    uint8_t action;      /* Action to take at waypoint */
} WaypointEvt;

/* CAN-FD message event */
typedef struct {
    QEvt super;
    uint32_t id;
    uint8_t dlc;
    uint8_t data[64];
} CANFDEvt;

/* Android command event */
typedef struct {
    QEvt super;
    uint8_t cmd_type;
    uint8_t payload[32];
    uint8_t payload_len;
} AndroidCmdEvt;

/*==========================================================================*/
/* Active Objects (declared in respective source files) */
/*==========================================================================*/

extern QActive * const AO_MotorCtrl;
extern QActive * const AO_TurretCtrl;
extern QActive * const AO_PathPlanner;
extern QActive * const AO_SensorFusion;
extern QActive * const AO_AndroidComm;
extern QActive * const AO_Supervisor;

/*==========================================================================*/
/* Function Prototypes */
/*==========================================================================*/

/* Active Object constructors */
void MotorCtrl_ctor(void);
void TurretCtrl_ctor(void);
void PathPlanner_ctor(void);
void SensorFusion_ctor(void);
void AndroidComm_ctor(void);
void Supervisor_ctor(void);

#endif /* ROBOT_H */
