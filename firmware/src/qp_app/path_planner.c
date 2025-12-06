/*
 * Path Planner Active Object
 * FRDM-MCXN947 4WD Robot
 * 
 * Handles waypoint navigation and trajectory following.
 */

#include "robot.h"
#include <math.h>

/*==========================================================================*/
/* Path Planner Active Object */
/*==========================================================================*/

#define MAX_WAYPOINTS   32

typedef struct {
    int32_t latitude;
    int32_t longitude;
    uint8_t action;
} Waypoint;

typedef struct {
    QActive super;
    
    /* Waypoint list */
    Waypoint waypoints[MAX_WAYPOINTS];
    uint8_t waypoint_count;
    uint8_t current_waypoint;
    
    /* Current position (from sensor fusion) */
    int32_t current_lat;
    int32_t current_lon;
    uint16_t current_heading;
    
    /* Navigation state */
    bool navigating;
    bool obstacle_detected;
    
    /* Control outputs */
    int16_t desired_speed;
    int16_t desired_heading;
    
    /* Time event for periodic update */
    QTimeEvt timeEvt;
} PathPlanner;

/* Navigation parameters */
#define WAYPOINT_RADIUS     50000   /* 5 meters in 1e-7 degrees */
#define MAX_SPEED           1000    /* Max RPM */
#define TURN_SPEED          500     /* Turning RPM */

/* Local instance */
static PathPlanner l_pathPlanner;

/* Global pointer */
QActive * const AO_PathPlanner = &l_pathPlanner.super;

/*==========================================================================*/
/* State Handlers */
/*==========================================================================*/

static QState PathPlanner_initial(PathPlanner * const me, QEvt const * const e);
static QState PathPlanner_idle(PathPlanner * const me, QEvt const * const e);
static QState PathPlanner_navigating(PathPlanner * const me, QEvt const * const e);
static QState PathPlanner_avoiding(PathPlanner * const me, QEvt const * const e);

/*==========================================================================*/
/* Helper Functions */
/*==========================================================================*/

static int32_t PathPlanner_distance(int32_t lat1, int32_t lon1, 
                                     int32_t lat2, int32_t lon2) {
    /* Simple Euclidean distance (good enough for short distances) */
    int32_t dlat = lat2 - lat1;
    int32_t dlon = lon2 - lon1;
    /* Approximate: sqrt(dlat^2 + dlon^2) */
    int32_t abs_dlat = dlat < 0 ? -dlat : dlat;
    int32_t abs_dlon = dlon < 0 ? -dlon : dlon;
    return abs_dlat + abs_dlon;  /* Manhattan distance approximation */
}

static int16_t PathPlanner_bearing(int32_t lat1, int32_t lon1,
                                    int32_t lat2, int32_t lon2) {
    /* Calculate bearing in degrees * 100 */
    int32_t dlat = lat2 - lat1;
    int32_t dlon = lon2 - lon1;
    
    /* Simple atan2 approximation */
    if (dlon == 0 && dlat == 0) return 0;
    
    /* Using integer approximation */
    int16_t angle;
    if (dlat >= 0 && dlon >= 0) {
        angle = (int16_t)((dlon * 9000) / (dlat + dlon + 1));
    } else if (dlat < 0 && dlon >= 0) {
        angle = 9000 + (int16_t)((-dlat * 9000) / (-dlat + dlon + 1));
    } else if (dlat < 0 && dlon < 0) {
        angle = 18000 + (int16_t)((-dlon * 9000) / (-dlat - dlon + 1));
    } else {
        angle = 27000 + (int16_t)((dlat * 9000) / (dlat - dlon + 1));
    }
    
    return angle;
}

static void PathPlanner_updateMotors(PathPlanner * const me) {
    /* Calculate differential drive based on heading error */
    int16_t heading_error = me->desired_heading - me->current_heading;
    
    /* Normalize to -18000 to 18000 */
    while (heading_error > 18000) heading_error -= 36000;
    while (heading_error < -18000) heading_error += 36000;
    
    /* Calculate left and right motor speeds */
    int16_t base_speed = me->desired_speed;
    int16_t turn_factor = heading_error / 100;  /* Scale down */
    
    int16_t left_speed = base_speed + turn_factor;
    int16_t right_speed = base_speed - turn_factor;
    
    /* Clamp speeds */
    if (left_speed > MAX_SPEED) left_speed = MAX_SPEED;
    if (left_speed < -MAX_SPEED) left_speed = -MAX_SPEED;
    if (right_speed > MAX_SPEED) right_speed = MAX_SPEED;
    if (right_speed < -MAX_SPEED) right_speed = -MAX_SPEED;
    
    /* Send motor commands */
    MotorEvt *evt_fl = Q_NEW(MotorEvt, SIG_MOTOR_SET_SPEED);
    MotorEvt *evt_fr = Q_NEW(MotorEvt, SIG_MOTOR_SET_SPEED);
    MotorEvt *evt_rl = Q_NEW(MotorEvt, SIG_MOTOR_SET_SPEED);
    MotorEvt *evt_rr = Q_NEW(MotorEvt, SIG_MOTOR_SET_SPEED);
    
    if (evt_fl && evt_fr && evt_rl && evt_rr) {
        evt_fl->motor_id = 0; evt_fl->speed = left_speed;
        evt_fr->motor_id = 1; evt_fr->speed = right_speed;
        evt_rl->motor_id = 2; evt_rl->speed = left_speed;
        evt_rr->motor_id = 3; evt_rr->speed = right_speed;
        
        QActive_post(AO_MotorCtrl, &evt_fl->super, 0U);
        QActive_post(AO_MotorCtrl, &evt_fr->super, 0U);
        QActive_post(AO_MotorCtrl, &evt_rl->super, 0U);
        QActive_post(AO_MotorCtrl, &evt_rr->super, 0U);
    }
}

static void PathPlanner_stopMotors(PathPlanner * const me) {
    (void)me;
    
    MotorEvt *evt = Q_NEW(MotorEvt, SIG_MOTOR_STOP);
    if (evt) {
        QActive_post(AO_MotorCtrl, &evt->super, 0U);
    }
}

/*==========================================================================*/
/* State Machine Implementation */
/*==========================================================================*/

static QState PathPlanner_initial(PathPlanner * const me, QEvt const * const e) {
    (void)e;
    
    me->waypoint_count = 0;
    me->current_waypoint = 0;
    me->current_lat = 0;
    me->current_lon = 0;
    me->current_heading = 0;
    me->navigating = false;
    me->obstacle_detected = false;
    me->desired_speed = 0;
    me->desired_heading = 0;
    
    /* Arm periodic timer (100ms) */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC / 10, BSP_TICKS_PER_SEC / 10);
    
    return Q_TRAN(&PathPlanner_idle);
}

static QState PathPlanner_idle(PathPlanner * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("PathPlanner: IDLE\r\n");
            me->navigating = false;
            PathPlanner_stopMotors(me);
            status = Q_HANDLED();
            break;
            
        case SIG_PATH_UPDATE: {
            WaypointEvt const *evt = Q_EVT_CAST(WaypointEvt);
            if (me->waypoint_count < MAX_WAYPOINTS) {
                me->waypoints[me->waypoint_count].latitude = evt->latitude;
                me->waypoints[me->waypoint_count].longitude = evt->longitude;
                me->waypoints[me->waypoint_count].action = evt->action;
                me->waypoint_count++;
            }
            
            if (me->waypoint_count > 0) {
                me->current_waypoint = 0;
                status = Q_TRAN(&PathPlanner_navigating);
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_GPS_UPDATE: {
            GPSEvt const *evt = Q_EVT_CAST(GPSEvt);
            me->current_lat = evt->latitude;
            me->current_lon = evt->longitude;
            me->current_heading = evt->heading;
            status = Q_HANDLED();
            break;
        }
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState PathPlanner_navigating(PathPlanner * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("PathPlanner: NAVIGATING\r\n");
            me->navigating = true;
            status = Q_HANDLED();
            break;
            
        case SIG_GPS_UPDATE: {
            GPSEvt const *evt = Q_EVT_CAST(GPSEvt);
            me->current_lat = evt->latitude;
            me->current_lon = evt->longitude;
            me->current_heading = evt->heading;
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT: {
            if (me->current_waypoint >= me->waypoint_count) {
                /* All waypoints reached */
                status = Q_TRAN(&PathPlanner_idle);
            } else {
                Waypoint *wp = &me->waypoints[me->current_waypoint];
                
                /* Calculate distance and bearing to waypoint */
                int32_t dist = PathPlanner_distance(
                    me->current_lat, me->current_lon,
                    wp->latitude, wp->longitude);
                    
                if (dist < WAYPOINT_RADIUS) {
                    /* Waypoint reached */
                    me->current_waypoint++;
                    
                    /* Notify that waypoint was reached */
                    WaypointEvt *evt = Q_NEW(WaypointEvt, SIG_WAYPOINT_REACHED);
                    if (evt) {
                        evt->waypoint_id = me->current_waypoint - 1;
                        QActive_post(AO_Supervisor, &evt->super, 0U);
                    }
                } else {
                    /* Navigate to waypoint */
                    me->desired_heading = PathPlanner_bearing(
                        me->current_lat, me->current_lon,
                        wp->latitude, wp->longitude);
                    me->desired_speed = MAX_SPEED;
                    
                    PathPlanner_updateMotors(me);
                }
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_OBSTACLE_DETECTED:
            status = Q_TRAN(&PathPlanner_avoiding);
            break;
            
        case SIG_EMERGENCY_STOP:
            status = Q_TRAN(&PathPlanner_idle);
            break;
            
        default:
            status = Q_SUPER(&QHsm_top);
            break;
    }
    
    return status;
}

static QState PathPlanner_avoiding(PathPlanner * const me, QEvt const * const e) {
    QState status;
    
    switch (e->sig) {
        case Q_ENTRY_SIG:
            BSP_DEBUG_PRINT("PathPlanner: AVOIDING\r\n");
            me->obstacle_detected = true;
            /* Stop and turn */
            me->desired_speed = 0;
            PathPlanner_stopMotors(me);
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            /* Simple avoidance: turn right */
            me->desired_heading += 4500;  /* Turn 45 degrees */
            if (me->desired_heading > 36000) {
                me->desired_heading -= 36000;
            }
            me->desired_speed = TURN_SPEED;
            PathPlanner_updateMotors(me);
            
            /* Check if obstacle is cleared (simplified) */
            if (!me->obstacle_detected) {
                status = Q_TRAN(&PathPlanner_navigating);
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_VISION_OBSTACLE:
            /* Still detecting obstacle */
            me->obstacle_detected = true;
            status = Q_HANDLED();
            break;
            
        case SIG_EMERGENCY_STOP:
            status = Q_TRAN(&PathPlanner_idle);
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

void PathPlanner_ctor(void) {
    PathPlanner *me = &l_pathPlanner;
    
    QActive_ctor(&me->super, (QStateHandler)&PathPlanner_initial);
    QTimeEvt_ctorX(&me->timeEvt, &me->super, SIG_TIMEOUT, 0U);
}
