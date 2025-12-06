/*
 * Supervisor Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 * 
 * Master control for system state, emergency stops, and coordination.
 */

#include "robot.hpp"

namespace Robot {

//==========================================================================
// Supervisor Active Object Class
//==========================================================================

class SupervisorAO : public QP::QActive {
private:
    enum class RobotState : std::uint8_t {
        INIT,
        IDLE,
        MANUAL,
        AUTONOMOUS,
        EMERGENCY,
        FAULT
    };
    
    // Error flags
    static constexpr std::uint32_t ERR_MOTOR_FAULT = (1U << 0);
    static constexpr std::uint32_t ERR_CANFD_ERROR = (1U << 1);
    static constexpr std::uint32_t ERR_GPS_LOST = (1U << 2);
    static constexpr std::uint32_t ERR_LOW_BATTERY = (1U << 5);
    
    // Battery thresholds (mV)
    static constexpr std::uint16_t BATTERY_LOW_MV = 22000;
    static constexpr std::uint16_t BATTERY_CRITICAL_MV = 20000;
    static constexpr std::uint16_t BATTERY_FULL_MV = 25200;
    
    // System state
    RobotState m_robotState;
    
    // Health monitoring
    std::uint16_t m_batteryVoltage;
    std::uint8_t m_batteryPercent;
    bool m_estopActive;
    bool m_androidConnected;
    
    // Error flags
    std::uint32_t m_errorFlags;
    
    // Statistics
    std::uint32_t m_uptimeSec;
    std::uint32_t m_waypointsReached;
    
    // Time event
    QP::QTimeEvt m_timeEvt;

public:
    SupervisorAO();
    
private:
    static QP::QState initial(SupervisorAO* const me, QP::QEvt const* const e);
    static QP::QState idle(SupervisorAO* const me, QP::QEvt const* const e);
    static QP::QState manual(SupervisorAO* const me, QP::QEvt const* const e);
    static QP::QState autonomous(SupervisorAO* const me, QP::QEvt const* const e);
    static QP::QState emergency(SupervisorAO* const me, QP::QEvt const* const e);
    
    void checkBattery();
    void checkEstop();
    void broadcastHeartbeat();
    void broadcastEmergencyStop();
    void updateLEDs();
};

// Local instance
static SupervisorAO l_supervisor;

// Global pointer
QP::QActive* const AO_Supervisor = &l_supervisor;

//==========================================================================
// Constructor
//==========================================================================

SupervisorAO::SupervisorAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_robotState(RobotState::INIT),
      m_batteryVoltage(0),
      m_batteryPercent(0),
      m_estopActive(false),
      m_androidConnected(false),
      m_errorFlags(0),
      m_uptimeSec(0),
      m_waypointsReached(0),
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

void SupervisorAO::checkBattery() {
    m_batteryVoltage = BSP::Adc::readBatteryMv();
    
    if (m_batteryVoltage >= BATTERY_FULL_MV) {
        m_batteryPercent = 100;
    } else if (m_batteryVoltage <= BATTERY_CRITICAL_MV) {
        m_batteryPercent = 0;
    } else {
        m_batteryPercent = static_cast<std::uint8_t>(
            (m_batteryVoltage - BATTERY_CRITICAL_MV) * 100 / 
            (BATTERY_FULL_MV - BATTERY_CRITICAL_MV));
    }
    
    if (m_batteryVoltage < BATTERY_LOW_MV) {
        m_errorFlags |= ERR_LOW_BATTERY;
    } else {
        m_errorFlags &= ~ERR_LOW_BATTERY;
    }
}

void SupervisorAO::checkEstop() {
    m_estopActive = BSP::isEstopActive();
}

void SupervisorAO::broadcastHeartbeat() {
    auto* evt = new QP::QEvt(SIG_HEARTBEAT);
    AO_MotorCtrl->post(evt, 0U);
    
    evt = new QP::QEvt(SIG_HEARTBEAT);
    AO_TurretCtrl->post(evt, 0U);
}

void SupervisorAO::broadcastEmergencyStop() {
    auto* evt = new QP::QEvt(SIG_EMERGENCY_STOP);
    AO_MotorCtrl->post(evt, 0U);
    
    evt = new QP::QEvt(SIG_EMERGENCY_STOP);
    AO_TurretCtrl->post(evt, 0U);
    
    evt = new QP::QEvt(SIG_EMERGENCY_STOP);
    AO_PathPlanner->post(evt, 0U);
}

void SupervisorAO::updateLEDs() {
    switch (m_robotState) {
        case RobotState::IDLE:
            BSP::Led::on(BSP::Led::GREEN);
            BSP::Led::off(BSP::Led::RED);
            BSP::Led::off(BSP::Led::BLUE);
            break;
        case RobotState::MANUAL:
            BSP::Led::off(BSP::Led::GREEN);
            BSP::Led::off(BSP::Led::RED);
            BSP::Led::on(BSP::Led::BLUE);
            break;
        case RobotState::AUTONOMOUS:
            BSP::Led::on(BSP::Led::GREEN);
            BSP::Led::off(BSP::Led::RED);
            BSP::Led::on(BSP::Led::BLUE);
            break;
        case RobotState::EMERGENCY:
            BSP::Led::off(BSP::Led::GREEN);
            BSP::Led::on(BSP::Led::RED);
            BSP::Led::off(BSP::Led::BLUE);
            break;
        default:
            BSP::Led::toggle(BSP::Led::RED);
            break;
    }
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState SupervisorAO::initial(SupervisorAO* const me, QP::QEvt const* const e) {
    (void)e;
    
    // Initialize CAN-FD via C driver
    BSP::CanFD canfd0(0);
    canfd0.init();
    
    // Initialize PWM
    BSP::Pwm::init();
    
    // Arm periodic timer (1 second heartbeat)
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC, BSP::TICKS_PER_SEC);
    
    return me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
}

QP::QState SupervisorAO::idle(SupervisorAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("Supervisor: IDLE\r\n");
            me->m_robotState = RobotState::IDLE;
            me->updateLEDs();
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            ++me->m_uptimeSec;
            me->checkBattery();
            me->checkEstop();
            me->broadcastHeartbeat();
            me->updateLEDs();
            
            if (me->m_estopActive) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&emergency));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_ANDROID_CMD: {
            auto const* evt = static_cast<AndroidCmdEvt const*>(e);
            switch (evt->cmdType) {
                case 1:  // Manual mode
                    status = me->tran(reinterpret_cast<QP::QStateHandler>(&manual));
                    break;
                case 2:  // Autonomous mode
                    status = me->tran(reinterpret_cast<QP::QStateHandler>(&autonomous));
                    break;
                default:
                    status = Q_HANDLED();
                    break;
            }
            break;
        }
            
        case SIG_WAYPOINT_REACHED:
            ++me->m_waypointsReached;
            status = Q_HANDLED();
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState SupervisorAO::manual(SupervisorAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("Supervisor: MANUAL\r\n");
            me->m_robotState = RobotState::MANUAL;
            me->updateLEDs();
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            ++me->m_uptimeSec;
            me->checkBattery();
            me->checkEstop();
            
            if (me->m_estopActive || me->m_batteryVoltage < BATTERY_CRITICAL_MV) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&emergency));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_ANDROID_CMD: {
            auto const* evt = static_cast<AndroidCmdEvt const*>(e);
            if (evt->cmdType == 0) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            } else if (evt->cmdType == 2) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&autonomous));
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState SupervisorAO::autonomous(SupervisorAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("Supervisor: AUTONOMOUS\r\n");
            me->m_robotState = RobotState::AUTONOMOUS;
            me->updateLEDs();
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            ++me->m_uptimeSec;
            me->checkBattery();
            me->checkEstop();
            
            if (me->m_estopActive || me->m_batteryVoltage < BATTERY_CRITICAL_MV) {
                me->broadcastEmergencyStop();
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&emergency));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_WAYPOINT_REACHED:
            ++me->m_waypointsReached;
            status = Q_HANDLED();
            break;
            
        case SIG_ANDROID_CMD: {
            auto const* evt = static_cast<AndroidCmdEvt const*>(e);
            if (evt->cmdType == 0) {
                me->broadcastEmergencyStop();
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            } else if (evt->cmdType == 1) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&manual));
            } else {
                status = Q_HANDLED();
            }
            break;
        }
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState SupervisorAO::emergency(SupervisorAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("Supervisor: EMERGENCY\r\n");
            me->m_robotState = RobotState::EMERGENCY;
            me->broadcastEmergencyStop();
            me->updateLEDs();
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            ++me->m_uptimeSec;
            me->checkBattery();
            me->checkEstop();
            me->updateLEDs();
            
            // Check if emergency condition cleared
            if (!me->m_estopActive && me->m_batteryVoltage >= BATTERY_LOW_MV) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

} // namespace Robot
