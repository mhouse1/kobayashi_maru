/*
 * Robot Main Entry Point (C++)
 * FRDM-MCXN947 4WD Robot with QP/C++ Framework
 * 
 * Architecture:
 * - C++ for: MCU modules (QP framework), middleware, turret control
 * - C for: Low-level drivers, ISRs, performance-critical routines
 */

#include "robot.hpp"
#include <array>

//==========================================================================
// Event Queues for Active Objects
//==========================================================================

static std::array<QP::QEvt const*, 16> motorCtrlQueueSto;
static std::array<QP::QEvt const*, 16> turretCtrlQueueSto;
static std::array<QP::QEvt const*, 32> pathPlannerQueueSto;
static std::array<QP::QEvt const*, 32> sensorFusionQueueSto;
static std::array<QP::QEvt const*, 64> androidCommQueueSto;
static std::array<QP::QEvt const*, 16> supervisorQueueSto;

//==========================================================================
// Event Pool
//==========================================================================

static std::array<std::uint8_t, 4096> eventPoolSto;

//==========================================================================
// Main Function
//==========================================================================

int main() {
    // Initialize Board Support Package (C drivers)
    BSP::init();
    
    // Initialize QP/C++ framework
    QP::init();
    
    // Initialize event pool
    QP::poolInit(eventPoolSto.data(), eventPoolSto.size(), 
                 sizeof(Robot::CANFDEvt));  // Largest event
    
    // Start Active Objects
    Robot::AO_MotorCtrl->start(
        static_cast<std::uint8_t>(Robot::Priority::MOTOR_CTRL),
        motorCtrlQueueSto.data(), motorCtrlQueueSto.size(),
        nullptr, 0U, nullptr);
    
    Robot::AO_TurretCtrl->start(
        static_cast<std::uint8_t>(Robot::Priority::TURRET_CTRL),
        turretCtrlQueueSto.data(), turretCtrlQueueSto.size(),
        nullptr, 0U, nullptr);
    
    Robot::AO_PathPlanner->start(
        static_cast<std::uint8_t>(Robot::Priority::PATH_PLANNER),
        pathPlannerQueueSto.data(), pathPlannerQueueSto.size(),
        nullptr, 0U, nullptr);
    
    Robot::AO_SensorFusion->start(
        static_cast<std::uint8_t>(Robot::Priority::SENSOR_FUSION),
        sensorFusionQueueSto.data(), sensorFusionQueueSto.size(),
        nullptr, 0U, nullptr);
    
    Robot::AO_AndroidComm->start(
        static_cast<std::uint8_t>(Robot::Priority::ANDROID_COMM),
        androidCommQueueSto.data(), androidCommQueueSto.size(),
        nullptr, 0U, nullptr);
    
    Robot::AO_Supervisor->start(
        static_cast<std::uint8_t>(Robot::Priority::SUPERVISOR),
        supervisorQueueSto.data(), supervisorQueueSto.size(),
        nullptr, 0U, nullptr);
    
    // Start BSP (enables interrupts via C driver)
    BSP::start();
    
    // Run QP/C++ framework (never returns)
    return QP::run();
}
