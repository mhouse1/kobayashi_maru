/*
 * Sensor Fusion Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Combines GPS, IMU, and vision data for accurate position estimation.
 */

#include "robot.h"

/*==========================================================================*/
/* Sensor Fusion Active Object */
/*==========================================================================*/

typedef struct {
    QActive super;
    
    /* GPS data */
    int32_t gps_lat;
    int32_t gps_lon;
    int32_t gps_alt;
    uint16_t gps_heading;
    uint16_t gps_speed;
    uint8_t gps_fix;
    
    /* IMU data */
    int16_t accel_x, accel_y, accel_z;
    int16_t gyro_x, gyro_y, gyro_z;
    int16_t mag_x, mag_y, mag_z;
    
    /* Fused position estimate */
    int32_t est_lat;
    int32_t est_lon;
    int16_t est_heading;
    int16_t est_speed;
    int16_t est_pitch;
    int16_t est_roll;
    
    /* Filter state */
    uint32_t last_update;
    bool gps_valid;
    bool imu_valid;
    
    /* Time event for periodic update */
    QTimeEvt timeEvt;
} SensorFusion;

/* Local instance */
static SensorFusion l_sensorFusion;

/* Global pointer */
QActive * const AO_SensorFusion = &l_sensorFusion.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState SensorFusion_initial(SensorFusion * const me, QEvt const * const e);
static QState SensorFusion_running(SensorFusion * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static void SensorFusion_updateEstimate(SensorFusion * const me) {
    /* Simple complementary filter for sensor fusion */
    
    if (me->gps_valid) {
        /* Use GPS as primary position source */
        me->est_lat = me->gps_lat;
        me->est_lon = me->gps_lon;
        me->est_speed = (int16_t)me->gps_speed;
    }
    
    if (me->imu_valid) {
        /* Calculate heading from magnetometer */
        /* Simplified: atan2(mag_y, mag_x) */
        int32_t heading = 0;
        if (me->mag_x != 0 || me->mag_y != 0) {
            /* Simple heading calculation */
            if (me->mag_x > 0) {
                heading = (int32_t)me->mag_y * 9000 / (me->mag_x + 1);
            } else if (me->mag_x < 0) {
                heading = 18000 + (int32_t)me->mag_y * 9000 / (-me->mag_x + 1);
            } else {
                heading = me->mag_y > 0 ? 9000 : 27000;
            }
        }
        
        /* Complementary filter: 90% GPS heading, 10% magnetometer */
        if (me->gps_valid && me->gps_speed > 100) {
            me->est_heading = (int16_t)((me->gps_heading * 9 + heading) / 10);
        } else {
            me->est_heading = (int16_t)heading;
        }
        
        /* Calculate pitch and roll from accelerometer */
        /* pitch = atan2(accel_x, sqrt(accel_y^2 + accel_z^2)) */
        /* roll = atan2(accel_y, accel_z) */
        
        int32_t accel_z_safe = me->accel_z != 0 ? me->accel_z : 1;
        me->est_pitch = (int16_t)((int32_t)me->accel_x * 9000 / (1000 + 1));
        me->est_roll = (int16_t)((int32_t)me->accel_y * 9000 / accel_z_safe);
    }
    
    me->last_update = BSP_get_tick();
}

static void SensorFusion_publishState(SensorFusion * const me) {
    /* Publish fused GPS data to path planner */
    GPSEvt *gps_evt = Q_NEW(GPSEvt, SIG_GPS_UPDATE);
    if (gps_evt) {
        gps_evt->latitude = me->est_lat;
        gps_evt->longitude = me->est_lon;
        gps_evt->altitude = me->gps_alt;
        gps_evt->heading = (uint16_t)me->est_heading;
        gps_evt->speed = (uint16_t)me->est_speed;
        gps_evt->satellites = 0;
        gps_evt->fix_quality = me->gps_fix;
        
        QActive_post(AO_PathPlanner, &gps_evt->super, 0U);
    }
    
    /* Publish IMU data to supervisor for monitoring */
    IMUEvt *imu_evt = Q_NEW(IMUEvt, SIG_IMU_UPDATE);
    if (imu_evt) {
        imu_evt->accel_x = me->accel_x;
        imu_evt->accel_y = me->accel_y;
        imu_evt->accel_z = me->accel_z;
        imu_evt->gyro_x = me->gyro_x;
        imu_evt->gyro_y = me->gyro_y;
        imu_evt->gyro_z = me->gyro_z;
        imu_evt->mag_x = me->mag_x;
        imu_evt->mag_y = me->mag_y;
        imu_evt->mag_z = me->mag_z;
        
        QActive_post(AO_Supervisor, &imu_evt->super, 0U);
    }
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState SensorFusion_initial(SensorFusion * const me, QEvt const * const e) {
    (void)e;
    
    /* Initialize all state variables */
    me->gps_lat = 0;
    me->gps_lon = 0;
    me->gps_alt = 0;
    me->gps_heading = 0;
    me->gps_speed = 0;
    me->gps_fix = 0;
    
    me->accel_x = 0;
    me->accel_y = 0;
    me->accel_z = 1000;  /* 1g in Z direction */
    me->gyro_x = 0;
    me->gyro_y = 0;
    me->gyro_z = 0;
    me->mag_x = 0;
    me->mag_y = 0;
    me->mag_z = 0;
    
    me->est_lat = 0;
    me->est_lon = 0;
    me->est_heading = 0;
    me->est_speed = 0;
    me->est_pitch = 0;
    me->est_roll = 0;
    
    me->last_update = 0;
    me->gps_valid = false;
    me->imu_valid = false;
    
    /* Arm periodic timer (20ms = 50Hz update rate) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC / 50, BSP_TICKS_PER_SEC / 50);
    
    return Q_TRAN(&SensorFusion_running);
}

static QState SensorFusion_running(SensorFusion * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("SensorFusion: RUNNING\r\n");
            status = Q_HANDLED();
            break;
            
        case SIG_GPS_UPDATE: {
            /* Receive GPS data from Android comm */
            GPSEvt const *evt = Q_EVT_CAST(GPSEvt);
            me->gps_lat = evt->latitude;
            me->gps_lon = evt->longitude;
            me->gps_alt = evt->altitude;
            me->gps_heading = evt->heading;
            me->gps_speed = evt->speed;
            me->gps_fix = evt->fix_quality;
            me->gps_valid = (evt->fix_quality > 0);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_IMU_UPDATE: {
            /* Receive IMU data from Android comm */
            IMUEvt const *evt = Q_EVT_CAST(IMUEvt);
            me->accel_x = evt->accel_x;
            me->accel_y = evt->accel_y;
            me->accel_z = evt->accel_z;
            me->gyro_x = evt->gyro_x;
            me->gyro_y = evt->gyro_y;
            me->gyro_z = evt->gyro_z;
            me->mag_x = evt->mag_x;
            me->mag_y = evt->mag_y;
            me->mag_z = evt->mag_z;
            me->imu_valid = true;
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            /* Periodic sensor fusion update */
            SensorFusion_updateEstimate(me);
            SensorFusion_publishState(me);
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

void SensorFusion_ctor(void) {
    SensorFusion *me = &l_sensorFusion;
    
    QActive_ctor(&me->super, (QStateHandler)&SensorFusion_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
