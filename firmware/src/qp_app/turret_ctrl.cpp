/*
 * Turret Control Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 *
 * Controls pan/tilt turret via PWM servos using QP/C++ framework.
 *
 * REQ-005.1a: Turret control protocol preliminarily defined (initial spec)
 * REQ-005.2: Turret control code implemented
 * REQ-005.3: Turret tested on hardware (planned)
 */

#include "robot.hpp"
#include <algorithm>

namespace Robot {

//==========================================================================
// Turret Control Active Object Class
//==========================================================================

class TurretCtrlAO : public QP::QActive {
private:
    // Turret position state
    std::int16_t m_panSetpoint;     // Degrees * 100
    std::int16_t m_tiltSetpoint;    // Degrees * 100
    std::int16_t m_panActual;       // Degrees * 100
    std::int16_t m_tiltActual;      // Degrees * 100
    
    // Turret speed
    std::uint16_t m_panSpeed;       // Degrees/sec
    std::uint16_t m_tiltSpeed;      // Degrees/sec
    
    // Control flags
    bool m_enabled;
    bool m_trackingMode;
    bool m_homed;
    
    // Vision target for tracking
    std::uint16_t m_targetX;
    std::uint16_t m_targetY;
    
    // Time event for periodic update
    QP::QTimeEvt m_timeEvt;

    // PWM channels
    static constexpr std::uint8_t PWM_CHANNEL_PAN = 0;
    static constexpr std::uint8_t PWM_CHANNEL_TILT = 1;
    
    // Turret limits (degrees * 100)
    static constexpr std::int16_t PAN_MIN = -18000;   // -180 degrees
    static constexpr std::int16_t PAN_MAX = 18000;    // +180 degrees
    static constexpr std::int16_t TILT_MIN = -4500;   // -45 degrees
    static constexpr std::int16_t TILT_MAX = 9000;    // +90 degrees

public:
    TurretCtrlAO();
    
private:
    // State handlers
    static QP::QState initial(TurretCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState idle(TurretCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState homing(TurretCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState positioning(TurretCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState tracking(TurretCtrlAO* const me, QP::QEvt const* const e);
    
    // Helper methods
    std::int16_t clampPan(std::int16_t value) const;
    std::int16_t clampTilt(std::int16_t value) const;
    void updatePWM();
    void moveToSetpoint();
};

// Local instance
static TurretCtrlAO l_turretCtrl;

// Global pointer
QP::QActive* const AO_TurretCtrl = &l_turretCtrl;

//==========================================================================
// Constructor
//==========================================================================

TurretCtrlAO::TurretCtrlAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_panSetpoint(0),
      m_tiltSetpoint(0),
      m_panActual(0),
      m_tiltActual(0),
      m_panSpeed(3000),   // 30 deg/sec
      m_tiltSpeed(2000),  // 20 deg/sec
      m_enabled(false),
      m_trackingMode(false),
      m_homed(false),
      m_targetX(0),
      m_targetY(0),
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

std::int16_t TurretCtrlAO::clampPan(std::int16_t value) const {
    return std::clamp(value, PAN_MIN, PAN_MAX);
}

std::int16_t TurretCtrlAO::clampTilt(std::int16_t value) const {
    return std::clamp(value, TILT_MIN, TILT_MAX);
}

void TurretCtrlAO::updatePWM() {
    BSP::Pwm::setServoAngle(PWM_CHANNEL_PAN, m_panActual);
    BSP::Pwm::setServoAngle(PWM_CHANNEL_TILT, m_tiltActual);
}

void TurretCtrlAO::moveToSetpoint() {
    // Move pan towards setpoint
    std::int16_t panStep = static_cast<std::int16_t>(m_panSpeed / 100);  // Step per 10ms update
    if (m_panActual < m_panSetpoint) {
        m_panActual = std::min(m_panSetpoint, static_cast<std::int16_t>(m_panActual + panStep));
    } else if (m_panActual > m_panSetpoint) {
        m_panActual = std::max(m_panSetpoint, static_cast<std::int16_t>(m_panActual - panStep));
    }
    
    // Move tilt towards setpoint
    std::int16_t tiltStep = static_cast<std::int16_t>(m_tiltSpeed / 100);
    if (m_tiltActual < m_tiltSetpoint) {
        m_tiltActual = std::min(m_tiltSetpoint, static_cast<std::int16_t>(m_tiltActual + tiltStep));
    } else if (m_tiltActual > m_tiltSetpoint) {
        m_tiltActual = std::max(m_tiltSetpoint, static_cast<std::int16_t>(m_tiltActual - tiltStep));
    }
    
    updatePWM();
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState TurretCtrlAO::initial(TurretCtrlAO* const me, QP::QEvt const* const e) {
    (void)e;
    
    // Initialize PWM
    BSP::Pwm::init();
    
    // Arm periodic timer (10ms)
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC / 100, BSP::TICKS_PER_SEC / 100);
    
    return me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
}

QP::QState TurretCtrlAO::idle(TurretCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("TurretCtrl: IDLE\r\n");
            me->m_enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&homing));
            break;
            
        case SIG_TURRET_SET_PAN:
        case SIG_TURRET_SET_TILT: {
            auto const* evt = static_cast<TurretEvt const*>(e);
            me->m_panSetpoint = me->clampPan(evt->panAngle);
            me->m_tiltSetpoint = me->clampTilt(evt->tiltAngle);
            if (evt->speed > 0) {
                me->m_panSpeed = evt->speed;
                me->m_tiltSpeed = evt->speed;
            }
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&positioning));
            break;
        }
            
        case SIG_TURRET_TRACK:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&tracking));
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState TurretCtrlAO::homing(TurretCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("TurretCtrl: HOMING\r\n");
            me->m_enabled = true;
            me->m_panSetpoint = 0;
            me->m_tiltSetpoint = 0;
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            me->moveToSetpoint();
            if (me->m_panActual == 0 && me->m_tiltActual == 0) {
                me->m_homed = true;
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
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

QP::QState TurretCtrlAO::positioning(TurretCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("TurretCtrl: POSITIONING\r\n");
            me->m_enabled = true;
            me->m_trackingMode = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_SET_PAN:
        case SIG_TURRET_SET_TILT: {
            auto const* evt = static_cast<TurretEvt const*>(e);
            me->m_panSetpoint = me->clampPan(evt->panAngle);
            me->m_tiltSetpoint = me->clampTilt(evt->tiltAngle);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            me->moveToSetpoint();
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&homing));
            break;
            
        case SIG_TURRET_TRACK:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&tracking));
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

QP::QState TurretCtrlAO::tracking(TurretCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("TurretCtrl: TRACKING\r\n");
            me->m_enabled = true;
            me->m_trackingMode = true;
            status = Q_HANDLED();
            break;
            
        case SIG_VISION_TARGET: {
            auto const* evt = static_cast<VisionEvt const*>(e);
            me->m_targetX = evt->targetX;
            me->m_targetY = evt->targetY;
            
            // Convert pixel coordinates to pan/tilt angles
            // Assume 640x480 resolution, center is (320, 240)
            std::int16_t panError = static_cast<std::int16_t>(320 - evt->targetX);
            std::int16_t tiltError = static_cast<std::int16_t>(240 - evt->targetY);
            
            // Simple proportional control
            me->m_panSetpoint = me->clampPan(me->m_panActual + panError * 10);
            me->m_tiltSetpoint = me->clampTilt(me->m_tiltActual + tiltError * 10);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            me->moveToSetpoint();
            status = Q_HANDLED();
            break;
            
        case SIG_TURRET_HOME:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&homing));
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
