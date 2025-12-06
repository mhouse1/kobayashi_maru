/*
 * Sensor Fusion Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 * 
 * Combines GPS, IMU, and vision data for accurate position estimation.
 */

#include "robot.hpp"
#include <cmath>

namespace Robot {

//==========================================================================
// Sensor Fusion Active Object Class
//==========================================================================

class SensorFusionAO : public QP::QActive {
private:
    // GPS data
    std::int32_t m_gpsLat;
    std::int32_t m_gpsLon;
    std::int32_t m_gpsAlt;
    std::uint16_t m_gpsHeading;
    std::uint16_t m_gpsSpeed;
    std::uint8_t m_gpsFix;
    
    // IMU data
    std::int16_t m_accelX, m_accelY, m_accelZ;
    std::int16_t m_gyroX, m_gyroY, m_gyroZ;
    std::int16_t m_magX, m_magY, m_magZ;
    
    // Fused position estimate
    std::int32_t m_estLat;
    std::int32_t m_estLon;
    std::int16_t m_estHeading;
    std::int16_t m_estSpeed;
    std::int16_t m_estPitch;
    std::int16_t m_estRoll;
    
    // Filter state
    std::uint32_t m_lastUpdate;
    bool m_gpsValid;
    bool m_imuValid;
    
    // Time event
    QP::QTimeEvt m_timeEvt;

public:
    SensorFusionAO();
    
private:
    static QP::QState initial(SensorFusionAO* const me, QP::QEvt const* const e);
    static QP::QState running(SensorFusionAO* const me, QP::QEvt const* const e);
    
    void updateEstimate();
    void publishState();
};

// Local instance
static SensorFusionAO l_sensorFusion;

// Global pointer
QP::QActive* const AO_SensorFusion = &l_sensorFusion;

//==========================================================================
// Constructor
//==========================================================================

SensorFusionAO::SensorFusionAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_gpsLat(0), m_gpsLon(0), m_gpsAlt(0),
      m_gpsHeading(0), m_gpsSpeed(0), m_gpsFix(0),
      m_accelX(0), m_accelY(0), m_accelZ(1000),
      m_gyroX(0), m_gyroY(0), m_gyroZ(0),
      m_magX(0), m_magY(0), m_magZ(0),
      m_estLat(0), m_estLon(0), m_estHeading(0),
      m_estSpeed(0), m_estPitch(0), m_estRoll(0),
      m_lastUpdate(0), m_gpsValid(false), m_imuValid(false),
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

void SensorFusionAO::updateEstimate() {
    if (m_gpsValid) {
        m_estLat = m_gpsLat;
        m_estLon = m_gpsLon;
        m_estSpeed = static_cast<std::int16_t>(m_gpsSpeed);
    }
    
    if (m_imuValid) {
        // Calculate heading from magnetometer
        std::int32_t heading = 0;
        if (m_magX != 0 || m_magY != 0) {
            heading = static_cast<std::int32_t>(std::atan2(m_magY, m_magX) * 18000.0 / 3.14159265);
        }
        
        // Complementary filter: 90% GPS heading, 10% magnetometer
        if (m_gpsValid && m_gpsSpeed > 100) {
            m_estHeading = static_cast<std::int16_t>((m_gpsHeading * 9 + heading) / 10);
        } else {
            m_estHeading = static_cast<std::int16_t>(heading);
        }
        
        // Calculate pitch and roll from accelerometer
        std::int32_t accelMag = static_cast<std::int32_t>(
            std::sqrt(m_accelX*m_accelX + m_accelY*m_accelY + m_accelZ*m_accelZ));
        if (accelMag > 0) {
            m_estPitch = static_cast<std::int16_t>(
                std::asin(static_cast<double>(m_accelX) / accelMag) * 18000.0 / 3.14159265);
            m_estRoll = static_cast<std::int16_t>(
                std::atan2(m_accelY, m_accelZ) * 18000.0 / 3.14159265);
        }
    }
    
    m_lastUpdate = BSP::getTick();
}

void SensorFusionAO::publishState() {
    // Publish fused GPS data to path planner
    auto* gpsEvt = new GPSEvt(SIG_GPS_UPDATE);
    gpsEvt->latitude = m_estLat;
    gpsEvt->longitude = m_estLon;
    gpsEvt->altitude = m_gpsAlt;
    gpsEvt->heading = static_cast<std::uint16_t>(m_estHeading);
    gpsEvt->speed = static_cast<std::uint16_t>(m_estSpeed);
    gpsEvt->fixQuality = m_gpsFix;
    AO_PathPlanner->post(gpsEvt, 0U);
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState SensorFusionAO::initial(SensorFusionAO* const me, QP::QEvt const* const e) {
    (void)e;
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC / 50, BSP::TICKS_PER_SEC / 50);
    return me->tran(reinterpret_cast<QP::QStateHandler>(&running));
}

QP::QState SensorFusionAO::running(SensorFusionAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("SensorFusion: RUNNING\r\n");
            status = Q_HANDLED();
            break;
            
        case SIG_GPS_UPDATE: {
            auto const* evt = static_cast<GPSEvt const*>(e);
            me->m_gpsLat = evt->latitude;
            me->m_gpsLon = evt->longitude;
            me->m_gpsAlt = evt->altitude;
            me->m_gpsHeading = evt->heading;
            me->m_gpsSpeed = evt->speed;
            me->m_gpsFix = evt->fixQuality;
            me->m_gpsValid = (evt->fixQuality > 0);
            status = Q_HANDLED();
            break;
        }
            
        case SIG_IMU_UPDATE: {
            auto const* evt = static_cast<IMUEvt const*>(e);
            me->m_accelX = evt->accelX;
            me->m_accelY = evt->accelY;
            me->m_accelZ = evt->accelZ;
            me->m_gyroX = evt->gyroX;
            me->m_gyroY = evt->gyroY;
            me->m_gyroZ = evt->gyroZ;
            me->m_magX = evt->magX;
            me->m_magY = evt->magY;
            me->m_magZ = evt->magZ;
            me->m_imuValid = true;
            status = Q_HANDLED();
            break;
        }
            
        case SIG_TIMEOUT:
            me->updateEstimate();
            me->publishState();
            status = Q_HANDLED();
            break;
            
        default:
            status = me->super(reinterpret_cast<QP::QStateHandler>(&QP::QHsm::top));
            break;
    }
    
    return status;
}

} // namespace Robot
