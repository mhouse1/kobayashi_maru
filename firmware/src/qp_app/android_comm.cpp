/*
 * Android Communication Active Object (C++)
 * FRDM-MCXN947 4WD Robot
 * 
 * Handles UART communication with Google Pixel 10 Pro.
 * Receives GPS, accelerometer, and vision data.
 */

#include "robot.hpp"
#include <cstring>
#include <cstdlib>
#include <array>

namespace Robot {

//==========================================================================
// Android Communication Active Object Class
//==========================================================================

class AndroidCommAO : public QP::QActive {
private:
    static constexpr std::size_t RX_BUFFER_SIZE = 256;
    static constexpr std::size_t TX_BUFFER_SIZE = 256;
    
    // Receive buffer
    std::array<char, RX_BUFFER_SIZE> m_rxBuffer;
    std::uint16_t m_rxIndex;
    
    // Transmit buffer
    std::array<char, TX_BUFFER_SIZE> m_txBuffer;
    
    // Communication state
    bool m_connected;
    std::uint32_t m_lastHeartbeat;
    std::uint32_t m_rxCount;
    std::uint32_t m_txCount;
    
    // UART interface
    BSP::Uart m_uart;
    
    // Time event
    QP::QTimeEvt m_timeEvt;

public:
    AndroidCommAO();
    
private:
    // State handlers
    static QP::QState initial(AndroidCommAO* const me, QP::QEvt const* const e);
    static QP::QState disconnected(AndroidCommAO* const me, QP::QEvt const* const e);
    static QP::QState connected(AndroidCommAO* const me, QP::QEvt const* const e);
    
    // Helper methods
    void sendMessage(const char* msg);
    void receiveChar(char c);
    void processMessage(const char* msg);
    void parseGPS(const char* data);
    void parseIMU(const char* data);
    void parseVision(const char* data);
    void parseCommand(const char* data);
};

// Local instance
static AndroidCommAO l_androidComm;

// Global pointer
QP::QActive* const AO_AndroidComm = &l_androidComm;

//==========================================================================
// Constructor
//==========================================================================

AndroidCommAO::AndroidCommAO()
    : QActive(reinterpret_cast<QP::QStateHandler>(&initial)),
      m_rxBuffer{},
      m_rxIndex(0),
      m_txBuffer{},
      m_connected(false),
      m_lastHeartbeat(0),
      m_rxCount(0),
      m_txCount(0),
      m_uart(1),  // UART channel 1 for Pixel
      m_timeEvt(this, SIG_TIMEOUT, 0U)
{
}

//==========================================================================
// Helper Methods
//==========================================================================

void AndroidCommAO::sendMessage(const char* msg) {
    m_uart.puts(msg);
    m_uart.puts("\r\n");
    ++m_txCount;
}

void AndroidCommAO::receiveChar(char c) {
    if (c == '\n' || c == '\r') {
        if (m_rxIndex > 0) {
            m_rxBuffer[m_rxIndex] = '\0';
            processMessage(m_rxBuffer.data());
            m_rxIndex = 0;
        }
    } else if (m_rxIndex < RX_BUFFER_SIZE - 1) {
        m_rxBuffer[m_rxIndex++] = c;
    }
}

void AndroidCommAO::processMessage(const char* msg) {
    ++m_rxCount;
    m_lastHeartbeat = BSP::getTick();
    
    if (std::strncmp(msg, "$GPS", 4) == 0) {
        parseGPS(msg);
    } else if (std::strncmp(msg, "$IMU", 4) == 0) {
        parseIMU(msg);
    } else if (std::strncmp(msg, "$VIS", 4) == 0) {
        parseVision(msg);
    } else if (std::strncmp(msg, "$CMD", 4) == 0) {
        parseCommand(msg);
    }
}

void AndroidCommAO::parseGPS(const char* data) {
    auto* evt = new GPSEvt(SIG_GPS_UPDATE);
    // Parse GPS data (simplified)
    AO_SensorFusion->post(evt, 0U);
}

void AndroidCommAO::parseIMU(const char* data) {
    auto* evt = new IMUEvt(SIG_IMU_UPDATE);
    // Parse IMU data (simplified)
    AO_SensorFusion->post(evt, 0U);
}

void AndroidCommAO::parseVision(const char* data) {
    auto* evt = new VisionEvt(SIG_VISION_TARGET);
    // Parse vision data (simplified)
    AO_TurretCtrl->post(evt, 0U);
}

void AndroidCommAO::parseCommand(const char* data) {
    auto* evt = new AndroidCmdEvt(SIG_ANDROID_CMD);
    // Parse command (simplified)
    AO_Supervisor->post(evt, 0U);
}

//==========================================================================
// State Machine Implementation
//==========================================================================

QP::QState AndroidCommAO::initial(AndroidCommAO* const me, QP::QEvt const* const e) {
    (void)e;
    
    me->m_uart.init();
    me->m_timeEvt.arm(BSP::TICKS_PER_SEC / 20, BSP::TICKS_PER_SEC / 20);
    
    return me->tran(reinterpret_cast<QP::QStateHandler>(&disconnected));
}

QP::QState AndroidCommAO::disconnected(AndroidCommAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("AndroidComm: DISCONNECTED\r\n");
            me->m_connected = false;
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT:
            while (me->m_uart.rxReady()) {
                me->receiveChar(me->m_uart.getchar());
            }
            if (me->m_rxCount > 0) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&connected));
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

QP::QState AndroidCommAO::connected(AndroidCommAO* const me, QP::QEvt const* const e) {
    QP::QState status;
    
    switch (e->sig) {
        case QP::Q_ENTRY_SIG:
            BSP::debugPrint("AndroidComm: CONNECTED\r\n");
            me->m_connected = true;
            me->sendMessage("$ACK,CONNECTED*");
            status = Q_HANDLED();
            break;
            
        case SIG_TIMEOUT: {
            while (me->m_uart.rxReady()) {
                me->receiveChar(me->m_uart.getchar());
            }
            
            std::uint32_t now = BSP::getTick();
            if ((now - me->m_lastHeartbeat) > 5000) {
                status = me->tran(reinterpret_cast<QP::QStateHandler>(&disconnected));
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

} // namespace Robot
