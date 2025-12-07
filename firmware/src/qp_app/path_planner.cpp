/*
 * Path Planner Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 * 
 * Handles waypoint navigation and trajectory following.
 */

#include "robot.hpp"
#include <algorithm>
#include <array>
#include <cmath>

namespace Robot {

//==========================================================================
// Path Planner Active Object Class
//==========================================================================

class PathPlannerAO : public QP::QActive {
private:
    static constexpr std::uint8_t MAX_WAYPOINTS = 32;
    static constexpr std::int32_t WAYPOINT_RADIUS = 50000;  // 5 meters
    static constexpr std::int16_t MAX_SPEED = 1000;         // Max RPM
    static constexpr std::int16_t TURN_SPEED = 500;         // Turning RPM
    
    struct Waypoint {
        std::int32_t latitude;
        std::int32_t longitude;
        std::uint8_t action;
    };
    
    // Waypoint list
    std::array<Waypoint, MAX_WAYPOINTS> m_waypoints;
    std::uint8_t m_waypointCount;
    std::uint8_t m_currentWaypoint;
    
    // Current position
    std::int32_t m_currentLat;
    std::int32_t m_currentLon;
    std::uint16_t m_currentHeading;
    
    // Navigation state
    bool m_navigating;
    bool m_obstacleDetected;
    
    // Control outputs
    std::int16_t m_desiredSpeed;
    std::int16_t m_desiredHeading;
    
    // Time event
    QP::QTimeEvt m_timeEvt;

public:
    PathPlannerAO();
    
private:
    // State handlers
    static QP::QState initial(PathPlannerAO* const me, QP::QEvt const* const e);
    static QP::QState idle(PathPlannerAO* const me, QP::QEvt const* const e);
    static QP::QState navigating(PathPlannerAO* const me, QP::QEvt const* const e);
    static QP::QState avoiding(PathPlannerAO* const me, QP::QEvt const* const e);
    
    // Helper methods
    std::int32_t distance(std::int32_t lat1, std::int32_t lon1,
                          std::int32_t lat2, std::int32_t lon2) const;
    std::int16_t bearing(std::int32_t lat1, std::int32_t lon1,
                         std::int32_t lat2, std::int32_t lon2) const;
    void updateMotors();
    void stopMotors();
};

// Local instance
static PathPlannerAO l_pathPlanner;

// Global pointer
QP::QActive* const AO_PathPlanner = &l_pathPlanner;

//==========================================================================
// Constructor
//==========================================================================

PathPlannerAO::PathPlannerAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_waypoints{},
      m_waypointCount(0),
      m_currentWaypoint(0),
      m_currentLat(0),
      m_currentLon(0),
      m_currentHeading(0),
      m_navigating(false),
      m_obstacleDetected(false),
      m_desiredSpeed(0),
      m_desiredHeading(0),
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

std::int32_t PathPlannerAO::distance(std::int32_t lat1, std::int32_t lon1,
                                      std::int32_t lat2, std::int32_t lon2) const {
    std::int32_t dlat = lat2 - lat1;
    std::int32_t dlon = lon2 - lon1;
    return std::abs(dlat) + std::abs(dlon);  // Manhattan distance
}

std::int16_t PathPlannerAO::bearing(std::int32_t lat1, std::int32_t lon1,
                                     std::int32_t lat2, std::int32_t lon2) const {
    std::int32_t dlat = lat2 - lat1;
    std::int32_t dlon = lon2 - lon1;
    if (dlat == 0 && dlon == 0) return 0;
    return static_cast<std::int16_t>(std::atan2(dlon, dlat) * 18000.0 / 3.14159265);
}

void PathPlannerAO::updateMotors() {
    std::int16_t headingError = m_desiredHeading - static_cast<std::int16_t>(m_currentHeading);
    while (headingError > 18000) headingError -= 36000;
    while (headingError < -18000) headingError += 36000;
    
    std::int16_t turnFactor = headingError / 100;
    std::int16_t leftSpeed = std::clamp<std::int16_t>(m_desiredSpeed + turnFactor, -MAX_SPEED, MAX_SPEED);
    std::int16_t rightSpeed = std::clamp<std::int16_t>(m_desiredSpeed - turnFactor, -MAX_SPEED, MAX_SPEED);
    
    // Send motor commands
    auto* evtFL = new MotorEvt(SIG_MOTOR_SET_SPEED);
    evtFL->motorId = 0; evtFL->speed = leftSpeed;
    AO_MotorCtrl->post(evtFL, 0U);
    
    auto* evtFR = new MotorEvt(SIG_MOTOR_SET_SPEED);
    evtFR->motorId = 1; evtFR->speed = rightSpeed;
    AO_MotorCtrl->post(evtFR, 0U);
    
    auto* evtRL = new MotorEvt(SIG_MOTOR_SET_SPEED);
    evtRL->motorId = 2; evtRL->speed = leftSpeed;
    AO_MotorCtrl->post(evtRL, 0U);
    
    auto* evtRR = new MotorEvt(SIG_MOTOR_SET_SPEED);
    evtRR->motorId = 3; evtRR->speed = rightSpeed;
    AO_MotorCtrl->post(evtRR, 0U);
}

void PathPlannerAO::stopMotors() {
    auto* evt = new MotorEvt(SIG_MOTOR_STOP);
    AO_MotorCtrl->post(evt, 0U);
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState PathPlannerAO::initial(PathPlannerAO* const me, QP::QEvt const* const e) {
    (void)e;
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC / 10, BSP::TICKS_PER_SEC / 10);
    return me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
}

QP::QState PathPlannerAO::idle(PathPlannerAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("PathPlanner: IDLE\r\n");
            me->m_navigating = false;
            me->stopMotors();
            status = Q_HANDLED();
            break;
            
        case SIG_PATH_UPDATE: {
            auto const* evt = static_cast<WaypointEvt const*>(e);
            if (me->m_waypointCount < MAX_WAYPOINTS) {
                me->m_waypoints[me->m_waypointCount].latitude = evt->latitude;
                me->m_waypoints[me->m_waypointCount].longitude = evt->longitude;
                me->m_waypoints[me->m_waypointCount].action = evt->action;
                ++me->m_waypointCount;
            }
            if (me->m_waypointCount > 0) {
                me->m_currentWaypoint = 0;
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&navigating));
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_GPS_UPDATE: {
            auto const* evt = static_cast<GPSEvt const*>(e);
            me->m_currentLat = evt->latitude;
            me->m_currentLon = evt->longitude;
            me->m_currentHeading = evt->heading;
            status = Q_HANDLED();
            break;
        }
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState PathPlannerAO::navigating(PathPlannerAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("PathPlanner: NAVIGATING\r\n");
            me->m_navigating = true;
            status = Q_HANDLED();
            break;
            
        case SIG_GPS_UPDATE: {
            auto const* evt = static_cast<GPSEvt const*>(e);
            me->m_currentLat = evt->latitude;
            me->m_currentLon = evt->longitude;
            me->m_currentHeading = evt->heading;
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT: {
            if (me->m_currentWaypoint >= me->m_waypointCount) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            } else {
                auto& wp = me->m_waypoints[me->m_currentWaypoint];
                std::int32_t dist = me->distance(me->m_currentLat, me->m_currentLon,
                                                  wp.latitude, wp.longitude);
                
                if (dist < WAYPOINT_RADIUS) {
                    ++me->m_currentWaypoint;
                    auto* evt = new WaypointEvt(SIG_WAYPOINT_REACHED);
                    evt->waypointId = me->m_currentWaypoint - 1;
                    AO_Supervisor->post(evt, 0U);
                } else {
                    me->m_desiredHeading = me->bearing(me->m_currentLat, me->m_currentLon,
                                                        wp.latitude, wp.longitude);
                    me->m_desiredSpeed = MAX_SPEED;
                    me->updateMotors();
                }
                status = Q_HANDLED();
            }
            break;
        }
            
        case SIG_OBSTACLE_DETECTED:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&avoiding));
            break;
            
        case SIG_EMERGENCY_STOP:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState PathPlannerAO::avoiding(PathPlannerAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("PathPlanner: AVOIDING\r\n");
            me->m_obstacleDetected = true;
            me->stopMotors();
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->m_desiredHeading += 4500;  // Turn 45 degrees
            if (me->m_desiredHeading > 36000) me->m_desiredHeading -= 36000;
            me->m_desiredSpeed = TURN_SPEED;
            me->updateMotors();
            
            if (!me->m_obstacleDetected) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&navigating));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_EMERGENCY_STOP:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

} // namespace Robot
