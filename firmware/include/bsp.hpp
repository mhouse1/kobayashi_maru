/*
 * Board Support Package (C++ Wrapper)
 * FRDM-MCXN947 Freedom Board
 * 
 * C++ wrapper classes for low-level C drivers.
 * Provides object-oriented interface for middleware integration.
 */

#ifndef BSP_HPP
#define BSP_HPP

#include <cstdint>
#include <string>

// Include C drivers with extern "C" linkage
extern "C" {
#include "bsp_drivers.h"
}

namespace BSP {

//==========================================================================
// System Configuration Constants
//==========================================================================

constexpr std::uint32_t TICKS_PER_SEC = BSP_TICKS_PER_SEC;
constexpr std::uint32_t CPU_FREQ = BSP_CPU_FREQ;

//==========================================================================
// LED Control Class
//==========================================================================

class Led {
public:
    enum Pin : std::uint8_t {
        RED = BSP_LED_RED_PIN,
        GREEN = BSP_LED_GREEN_PIN,
        BLUE = BSP_LED_BLUE_PIN
    };
    
    static void on(Pin led) { BSP_ledOn(static_cast<std::uint8_t>(led)); }
    static void off(Pin led) { BSP_ledOff(static_cast<std::uint8_t>(led)); }
    static void toggle(Pin led) { BSP_ledToggle(static_cast<std::uint8_t>(led)); }
};

//==========================================================================
// CAN-FD Interface Class
//==========================================================================

class CanFD {
public:
    static constexpr std::uint32_t BITRATE = BSP_CANFD_BITRATE;
    
    explicit CanFD(std::uint8_t channel) : m_channel(channel) {}
    
    bool init(std::uint32_t bitrate = BITRATE) {
        return BSP_canfdInit(m_channel, bitrate);
    }
    
    bool send(std::uint32_t id, const std::uint8_t* data, std::uint8_t dlc) {
        return BSP_canfdSend(m_channel, id, data, dlc);
    }
    
    bool receive(std::uint32_t& id, std::uint8_t* data, std::uint8_t& dlc) {
        return BSP_canfdReceive(m_channel, &id, data, &dlc);
    }
    
    void setFilter(std::uint32_t id, std::uint32_t mask) {
        BSP_canfdSetFilter(m_channel, id, mask);
    }
    
    bool txReady() const { return BSP_canfdTxReady(m_channel); }
    bool rxReady() const { return BSP_canfdRxReady(m_channel); }
    
private:
    std::uint8_t m_channel;
};

//==========================================================================
// UART Interface Class
//==========================================================================

class Uart {
public:
    static constexpr std::uint32_t DEFAULT_BAUDRATE = BSP_UART_BAUDRATE;
    
    explicit Uart(std::uint8_t channel) : m_channel(channel) {}
    
    bool init(std::uint32_t baudrate = DEFAULT_BAUDRATE) {
        return BSP_uartInit(m_channel, baudrate);
    }
    
    void putchar(char c) { BSP_uartPutchar(m_channel, c); }
    char getchar() { return BSP_uartGetchar(m_channel); }
    
    bool rxReady() const { return BSP_uartRxReady(m_channel); }
    bool txReady() const { return BSP_uartTxReady(m_channel); }
    
    void puts(const char* str) { BSP_uartPuts(m_channel, str); }
    
    // C++ string support
    void puts(const std::string& str) { puts(str.c_str()); }
    
private:
    std::uint8_t m_channel;
};

//==========================================================================
// PWM Interface Class (for Servo Control)
//==========================================================================

class Pwm {
public:
    static constexpr std::uint32_t SERVO_FREQ = BSP_PWM_FREQ;
    
    static void init() { BSP_pwmInit(); }
    
    static void setDuty(std::uint8_t channel, std::uint16_t duty) {
        BSP_pwmSetDuty(channel, duty);
    }
    
    static void setFreq(std::uint8_t channel, std::uint32_t freq) {
        BSP_pwmSetFreq(channel, freq);
    }
    
    // Servo-specific: angle to PWM duty conversion
    static void setServoAngle(std::uint8_t channel, std::int16_t angleCentideg) {
        // Convert angle (-18000 to +18000 centidegrees) to PWM (1000-2000us)
        std::int32_t duty = 1500 + (angleCentideg * 500 / 18000);
        if (duty < 1000) duty = 1000;
        if (duty > 2000) duty = 2000;
        setDuty(channel, static_cast<std::uint16_t>(duty));
    }
};

//==========================================================================
// ADC Interface Class
//==========================================================================

class Adc {
public:
    static void init() { BSP_adcInit(); }
    
    static std::uint16_t read(std::uint8_t channel) {
        return BSP_adcRead(channel);
    }
    
    // Battery voltage in millivolts (assuming voltage divider calibration)
    static std::uint16_t readBatteryMv() {
        return read(0);  // Channel 0 for battery
    }
};

//==========================================================================
// GPIO Interface Class
//==========================================================================

class Gpio {
public:
    static void init(std::uint8_t port, std::uint8_t pin, bool output) {
        BSP_gpioInit(port, pin, output);
    }
    
    static void set(std::uint8_t port, std::uint8_t pin) {
        BSP_gpioSet(port, pin);
    }
    
    static void clear(std::uint8_t port, std::uint8_t pin) {
        BSP_gpioClear(port, pin);
    }
    
    static void toggle(std::uint8_t port, std::uint8_t pin) {
        BSP_gpioToggle(port, pin);
    }
    
    static bool read(std::uint8_t port, std::uint8_t pin) {
        return BSP_gpioRead(port, pin);
    }
};

//==========================================================================
// System Functions
//==========================================================================

inline void init() { BSP_init(); }
inline void start() { BSP_start(); }
inline std::uint32_t getTick() { return BSP_getTick(); }
inline void delayMs(std::uint32_t ms) { BSP_delayMs(ms); }
inline bool isEstopActive() { return BSP_isEstopActive(); }

//==========================================================================
// Debug Output
//==========================================================================

#ifdef DEBUG
    inline void debugPrint(const char* msg) { BSP_uartPuts(0, msg); }
#else
    inline void debugPrint(const char*) {}
#endif

} // namespace BSP

#endif // BSP_HPP
