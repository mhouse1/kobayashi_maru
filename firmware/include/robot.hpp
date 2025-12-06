/*
 * Robot Main Header - QP/C++ Framework Integration
 * FRDM-MCXN947 Freedom Board + Google Pixel 10 Pro Architecture
 * 
 * This is the main header for the heavy duty 4WD robot firmware
 * using the Quantum QP/C++ Real-Time Embedded Framework.
 * 
 * Architecture:
 * - C++ for: MCU modules (QP framework), middleware, turret control,
 *            high-level motion control
 * - C for: Low-level drivers, ISRs, performance-critical routines
 */

#ifndef ROBOT_HPP
#define ROBOT_HPP

#include "qpcpp.hpp"
#include "bsp.hpp"

#include <cstdint>
#include <array>

namespace Robot {

//==========================================================================
// Robot Configuration
//==========================================================================

constexpr std::uint8_t NUM_WHEELS = 4;

// CAN-FD Node IDs
namespace CanFdNode {
    constexpr std::uint32_t MASTER = 0x001;
    constexpr std::uint32_t MOTOR_FL = 0x100;
    constexpr std::uint32_t MOTOR_FR = 0x101;
    constexpr std::uint32_t MOTOR_RL = 0x102;
    constexpr std::uint32_t MOTOR_RR = 0x103;
    constexpr std::uint32_t TURRET = 0x200;
    constexpr std::uint32_t GPS = 0x300;
    constexpr std::uint32_t IMU = 0x301;
    constexpr std::uint32_t VISION = 0x400;
}

//==========================================================================
// Active Object Priorities (QP framework)
//==========================================================================

enum class Priority : std::uint8_t {
    IDLE = 0,
    MOTOR_CTRL,
    TURRET_CTRL,
    PATH_PLANNER,
    SENSOR_FUSION,
    ANDROID_COMM,
    SUPERVISOR,
    MAX
};

//==========================================================================
// Event Signals
//==========================================================================

enum Signal : std::uint16_t {
    // System signals
    SIG_TIMEOUT = QP::Q_USER_SIG,
    SIG_HEARTBEAT,
    SIG_EMERGENCY_STOP,
    
    // Motor control signals
    SIG_MOTOR_SET_SPEED,
    SIG_MOTOR_SET_POSITION,
    SIG_MOTOR_STOP,
    SIG_MOTOR_STATUS,
    
    // Turret control signals
    SIG_TURRET_SET_PAN,
    SIG_TURRET_SET_TILT,
    SIG_TURRET_HOME,
    SIG_TURRET_TRACK,
    SIG_TURRET_STATUS,
    
    // Navigation signals
    SIG_GPS_UPDATE,
    SIG_IMU_UPDATE,
    SIG_WAYPOINT_REACHED,
    SIG_PATH_UPDATE,
    SIG_OBSTACLE_DETECTED,
    
    // Vision signals
    SIG_VISION_TARGET,
    SIG_VISION_OBSTACLE,
    SIG_VISION_FRAME,
    
    // Android/Pixel 10 Pro communication signals
    SIG_ANDROID_CMD,
    SIG_ANDROID_GPS,
    SIG_ANDROID_ACCEL,
    SIG_ANDROID_VISION,
    SIG_ANDROID_STATUS,
    
    // CAN-FD signals
    SIG_CANFD_RX,
    SIG_CANFD_TX_DONE,
    SIG_CANFD_ERROR,
    
    SIG_MAX
};

//==========================================================================
// Event Classes
//==========================================================================

// Motor control event
class MotorEvt : public QP::QEvt {
public:
    std::uint8_t motorId;
    std::int16_t speed;      // RPM, negative for reverse
    std::int32_t position;   // Encoder ticks
    
    explicit MotorEvt(std::uint16_t sig = 0) 
        : QEvt(sig), motorId(0), speed(0), position(0) {}
};

// Turret control event
class TurretEvt : public QP::QEvt {
public:
    std::int16_t panAngle;   // Degrees * 100
    std::int16_t tiltAngle;  // Degrees * 100
    std::uint16_t speed;     // Degrees/sec
    
    explicit TurretEvt(std::uint16_t sig = 0)
        : QEvt(sig), panAngle(0), tiltAngle(0), speed(0) {}
};

// GPS data event (from Pixel 10 Pro)
class GPSEvt : public QP::QEvt {
public:
    std::int32_t latitude;    // Degrees * 1e7
    std::int32_t longitude;   // Degrees * 1e7
    std::int32_t altitude;    // mm above sea level
    std::uint16_t heading;    // Degrees * 100
    std::uint16_t speed;      // mm/s
    std::uint8_t satellites;
    std::uint8_t fixQuality;
    
    explicit GPSEvt(std::uint16_t sig = 0)
        : QEvt(sig), latitude(0), longitude(0), altitude(0),
          heading(0), speed(0), satellites(0), fixQuality(0) {}
};

// IMU/Accelerometer event (from Pixel 10 Pro)
class IMUEvt : public QP::QEvt {
public:
    std::int16_t accelX;     // mg
    std::int16_t accelY;     // mg
    std::int16_t accelZ;     // mg
    std::int16_t gyroX;      // mdps (milli-degrees per second)
    std::int16_t gyroY;      // mdps
    std::int16_t gyroZ;      // mdps
    std::int16_t magX;       // mGauss
    std::int16_t magY;       // mGauss
    std::int16_t magZ;       // mGauss
    
    explicit IMUEvt(std::uint16_t sig = 0)
        : QEvt(sig), accelX(0), accelY(0), accelZ(0),
          gyroX(0), gyroY(0), gyroZ(0),
          magX(0), magY(0), magZ(0) {}
};

// Vision event (from Pixel 10 Pro)
class VisionEvt : public QP::QEvt {
public:
    std::uint16_t targetX;      // Pixel X coordinate
    std::uint16_t targetY;      // Pixel Y coordinate
    std::uint16_t targetWidth;  // Bounding box width
    std::uint16_t targetHeight; // Bounding box height
    std::uint8_t targetClass;   // Object classification
    std::uint8_t confidence;    // Detection confidence 0-100
    
    explicit VisionEvt(std::uint16_t sig = 0)
        : QEvt(sig), targetX(0), targetY(0), targetWidth(0),
          targetHeight(0), targetClass(0), confidence(0) {}
};

// Path waypoint event
class WaypointEvt : public QP::QEvt {
public:
    std::int32_t latitude;
    std::int32_t longitude;
    std::uint8_t waypointId;
    std::uint8_t action;      // Action to take at waypoint
    
    explicit WaypointEvt(std::uint16_t sig = 0)
        : QEvt(sig), latitude(0), longitude(0), waypointId(0), action(0) {}
};

// CAN-FD message event
class CANFDEvt : public QP::QEvt {
public:
    std::uint32_t id;
    std::uint8_t dlc;
    std::array<std::uint8_t, 64> data;
    
    explicit CANFDEvt(std::uint16_t sig = 0)
        : QEvt(sig), id(0), dlc(0), data{} {}
};

// Android command event
class AndroidCmdEvt : public QP::QEvt {
public:
    std::uint8_t cmdType;
    std::array<std::uint8_t, 32> payload;
    std::uint8_t payloadLen;
    
    explicit AndroidCmdEvt(std::uint16_t sig = 0)
        : QEvt(sig), cmdType(0), payload{}, payloadLen(0) {}
};

//==========================================================================
// Active Object Forward Declarations
//==========================================================================

class MotorCtrl;
class TurretCtrl;
class PathPlanner;
class SensorFusion;
class AndroidComm;
class Supervisor;

//==========================================================================
// Global Active Object Pointers
//==========================================================================

extern QP::QActive* const AO_MotorCtrl;
extern QP::QActive* const AO_TurretCtrl;
extern QP::QActive* const AO_PathPlanner;
extern QP::QActive* const AO_SensorFusion;
extern QP::QActive* const AO_AndroidComm;
extern QP::QActive* const AO_Supervisor;

} // namespace Robot

#endif // ROBOT_HPP
