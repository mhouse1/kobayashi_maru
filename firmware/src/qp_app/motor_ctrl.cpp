/*
 * Motor Control Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 * 
 * Controls all four wheel motors via CAN-FD bus using QP/C++ framework.
 */

#include "robot.hpp"
#include <array>

namespace Robot {

//==========================================================================
// Motor Control Active Object Class
//==========================================================================

class MotorCtrlAO : public QP::QActive {
private:
    // Motor states
    std::array<std::int16_t, NUM_WHEELS> m_motorSpeed;
    std::array<std::int32_t, NUM_WHEELS> m_motorPosition;
    std::array<std::uint8_t, NUM_WHEELS> m_motorStatus;
    
    // Control parameters
    bool m_enabled;
    bool m_emergencyStop;
    
    // CAN-FD interface
    BSP::CanFD m_canfd;
    
    // Time event for periodic status update
    QP::QTimeEvt m_timeEvt;

public:
    MotorCtrlAO();
    
private:
    // State handlers
    static QP::QState initial(MotorCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState idle(MotorCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState running(MotorCtrlAO* const me, QP::QEvt const* const e);
    static QP::QState stopped(MotorCtrlAO* const me, QP::QEvt const* const e);
    
    // Helper methods
    void sendMotorCommand(std::uint8_t motorId, std::int16_t speed);
    void stopAllMotors();
    std::uint32_t getMotorCanId(std::uint8_t motorId) const;
};

// Local instance
static MotorCtrlAO l_motorCtrl;

// Global pointer
QP::QActive* const AO_MotorCtrl = &l_motorCtrl;

//==========================================================================
// Constructor
//==========================================================================

MotorCtrlAO::MotorCtrlAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_motorSpeed{},
      m_motorPosition{},
      m_motorStatus{},
      m_enabled(false),
      m_emergencyStop(false),
      m_canfd(0),  // CAN-FD channel 0
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

std::uint32_t MotorCtrlAO::getMotorCanId(std::uint8_t motorId) const {
    switch (motorId) {
        case 0: return CanFdNode::MOTOR_FL;
        case 1: return CanFdNode::MOTOR_FR;
        case 2: return CanFdNode::MOTOR_RL;
        case 3: return CanFdNode::MOTOR_RR;
        default: return 0;
    }
}

void MotorCtrlAO::sendMotorCommand(std::uint8_t motorId, std::int16_t speed) {
    std::uint32_t canId = getMotorCanId(motorId);
    if (canId == 0) return;
    
    std::array<std::uint8_t, 8> data{};
    data[0] = 0x01;  // Command: set speed
    data[1] = static_cast<std::uint8_t>(speed & 0xFF);
    data[2] = static_cast<std::uint8_t>((speed >> 8) & 0xFF);
    data[3] = m_enabled ? 0x01 : 0x00;
    
    m_canfd.send(canId, data.data(), 8);
}

void MotorCtrlAO::stopAllMotors() {
    for (std::uint8_t i = 0; i < NUM_WHEELS; ++i) {
        m_motorSpeed[i] = 0;
        sendMotorCommand(i, 0);
    }
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState MotorCtrlAO::initial(MotorCtrlAO* const me, QP::QEvt const* const e) {
    (void)e;
    
    // Initialize motor states
    me->m_motorSpeed.fill(0);
    me->m_motorPosition.fill(0);
    me->m_motorStatus.fill(0);
    me->m_enabled = false;
    me->m_emergencyStop = false;
    
    // Initialize CAN-FD
    me->m_canfd.init();
    
    // Arm periodic timer for status updates (100ms)
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC / 10, BSP::TICKS_PER_SEC / 10);
    
    return me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
}

QP::QState MotorCtrlAO::idle(MotorCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("MotorCtrl: IDLE\r\n");
            me->m_enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED: {
            auto const* evt = static_cast<MotorEvt const*>(e);
            if (evt->motorId < NUM_WHEELS) {
                me->m_motorSpeed[evt->motorId] = evt->speed;
            }
            me->m_enabled = true;
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&running));
            break;
        }
        
        case SIG_TIMEOUT:
            // Periodic status check
            status = Q_HANDLED();
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState MotorCtrlAO::running(MotorCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("MotorCtrl: RUNNING\r\n");
            me->m_enabled = true;
            // Send current speeds to all motors
            for (std::uint8_t i = 0; i < NUM_WHEELS; ++i) {
                me->sendMotorCommand(i, me->m_motorSpeed[i]);
            }
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED: {
            auto const* evt = static_cast<MotorEvt const*>(e);
            if (evt->motorId < NUM_WHEELS) {
                me->m_motorSpeed[evt->motorId] = evt->speed;
                me->sendMotorCommand(evt->motorId, evt->speed);
            }
            status = Q_HANDLED();
            break;
        }
        
        case SIG_MOTOR_STOP:
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&stopped));
            break;
            
        case SIG_EMERGENCY_STOP:
            me->m_emergencyStop = true;
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&stopped));
            break;
            
        case SIG_TIMEOUT:
            // Periodic speed update to motors
            for (std::uint8_t i = 0; i < NUM_WHEELS; ++i) {
                me->sendMotorCommand(i, me->m_motorSpeed[i]);
            }
            status = Q_HANDLED();
            break;
            
        case SIG_CANFD_RX: {
            // Handle motor feedback
            auto const* evt = static_cast<CANFDEvt const*>(e);
            std::uint8_t motorId = 0xFF;
            
            // Determine which motor sent the message
            if (evt->id >= 0x110 && evt->id <= 0x113) {
                motorId = static_cast<std::uint8_t>(evt->id - 0x110);
            }
            
            if (motorId < NUM_WHEELS) {
                // Parse feedback data
                me->m_motorStatus[motorId] = evt->data[0];
                me->m_motorPosition[motorId] = 
                    static_cast<std::int32_t>(evt->data[1]) |
                    (static_cast<std::int32_t>(evt->data[2]) << 8) |
                    (static_cast<std::int32_t>(evt->data[3]) << 16) |
                    (static_cast<std::int32_t>(evt->data[4]) << 24);
            }
            status = Q_HANDLED();
            break;
        }
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

QP::QState MotorCtrlAO::stopped(MotorCtrlAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("MotorCtrl: STOPPED\r\n");
            me->stopAllMotors();
            me->m_enabled = false;
            status = Q_HANDLED();
            break;
            
        case SIG_MOTOR_SET_SPEED:
            if (!me->m_emergencyStop) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&running));
            } else {
                status = Q_HANDLED();
            }
            break;
            
        case SIG_HEARTBEAT:
            // Clear emergency stop on heartbeat
            me->m_emergencyStop = false;
            status = me->tran(reinterpret_cast<QP::QStateHandler>(&idle));
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

} // namespace Robot
