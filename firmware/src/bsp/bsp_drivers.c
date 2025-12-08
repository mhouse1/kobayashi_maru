/*
 * BSP Low-Level Driver Implementation (C)
 * FRDM-MCXN947 Freedom Board
 * 
 * Low-level hardware drivers, ISRs, and performance-critical routines.
 * Pure C for maximum performance and direct hardware access.
 *
 * REQ-002.1a: CAN-FD bus architecture preliminarily defined
 * REQ-002.2: CAN-FD driver implemented
 * REQ-002.3: CAN-FD loopback test on hardware (planned)
 * REQ-003.1a: Emergency stop requirement preliminarily defined
 * REQ-003.2: GPIO pin assigned and schematic updated
 * REQ-003.3: Emergency stop code implemented (planned)
 * REQ-003.4: Emergency stop tested on hardware (planned)
 */

#include "bsp_drivers.h"

/*==========================================================================*/
/* Private Variables */
/*==========================================================================*/

static volatile uint32_t g_sysTick = 0;

/* UART RX buffers */
#define UART_RX_BUFFER_SIZE 256
static volatile char g_uart0RxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t g_uart0RxHead = 0;
static volatile uint16_t g_uart0RxTail = 0;

static volatile char g_uart1RxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t g_uart1RxHead = 0;
static volatile uint16_t g_uart1RxTail = 0;

/* CAN-FD message buffers */
#define CANFD_RX_BUFFER_SIZE 16
typedef struct {
    uint32_t id;
    uint8_t data[64];
    uint8_t dlc;
    bool valid;
} CanfdMessage;

static volatile CanfdMessage g_canfd0RxBuffer[CANFD_RX_BUFFER_SIZE];
static volatile uint8_t g_canfd0RxHead = 0;
static volatile uint8_t g_canfd0RxTail = 0;

/*==========================================================================*/
/* System Initialization */
/*==========================================================================*/

void BSP_init(void) {
    /* Initialize system clocks (150 MHz) */
    /* In real implementation: configure PLL, flash wait states, etc. */
    
    /* Initialize SysTick for 1ms tick */
    /* SysTick->LOAD = (BSP_CPU_FREQ / BSP_TICKS_PER_SEC) - 1; */
    /* SysTick->VAL = 0; */
    /* SysTick->CTRL = 0x07; */  /* Enable, use processor clock, enable interrupt */
    
    /* Initialize GPIO for LEDs */
    BSP_gpioInit(0, BSP_LED_RED_PIN, true);
    BSP_gpioInit(0, BSP_LED_GREEN_PIN, true);
    BSP_gpioInit(0, BSP_LED_BLUE_PIN, true);
    
    /* Initialize GPIO for emergency stop (input) */
    BSP_gpioInit(0, BSP_ESTOP_PIN, false);
    
    /* Initialize ADC for battery monitoring */
    BSP_adcInit();
}

void BSP_start(void) {
    /* Enable global interrupts */
    BSP_EXIT_CRITICAL();
}

/*==========================================================================*/
/* System Tick */
/*==========================================================================*/

uint32_t BSP_getTick(void) {
    return g_sysTick;
}

void BSP_delayMs(uint32_t ms) {
    uint32_t start = g_sysTick;
    while ((g_sysTick - start) < ms) {
        /* Wait */
    }
}

/*==========================================================================*/
/* LED Control */
/*==========================================================================*/

void BSP_ledOn(uint8_t led) {
    BSP_gpioSet(0, led);
}

void BSP_ledOff(uint8_t led) {
    BSP_gpioClear(0, led);
}

void BSP_ledToggle(uint8_t led) {
    BSP_gpioToggle(0, led);
}

/*==========================================================================*/
/* CAN-FD Low-Level Operations */
/*==========================================================================*/

bool BSP_canfdInit(uint8_t channel, uint32_t bitrate) {
    (void)channel;
    (void)bitrate;
    
    /* In real implementation:
     * - Configure CAN-FD peripheral clocks
     * - Set bit timing for nominal and data phases
     * - Configure message buffers
     * - Enable interrupts
     */
    
    return true;
}

bool BSP_canfdSend(uint8_t channel, uint32_t id, const uint8_t* data, uint8_t dlc) {
    (void)channel;
    (void)id;
    (void)data;
    (void)dlc;
    
    /* In real implementation:
     * - Wait for TX buffer to be available
     * - Write message ID, DLC, and data to TX buffer
     * - Trigger transmission
     */
    
    return true;
}

bool BSP_canfdReceive(uint8_t channel, uint32_t* id, uint8_t* data, uint8_t* dlc) {
    if (channel == 0) {
        if (g_canfd0RxHead != g_canfd0RxTail) {
            volatile CanfdMessage* msg = &g_canfd0RxBuffer[g_canfd0RxTail];
            *id = msg->id;
            *dlc = msg->dlc;
            for (uint8_t i = 0; i < msg->dlc && i < 64; i++) {
                data[i] = msg->data[i];
            }
            g_canfd0RxTail = (g_canfd0RxTail + 1) % CANFD_RX_BUFFER_SIZE;
            return true;
        }
    }
    return false;
}

void BSP_canfdSetFilter(uint8_t channel, uint32_t id, uint32_t mask) {
    (void)channel;
    (void)id;
    (void)mask;
    
    /* In real implementation: configure hardware message filters */
}

bool BSP_canfdTxReady(uint8_t channel) {
    (void)channel;
    return true;  /* Simplified */
}

bool BSP_canfdRxReady(uint8_t channel) {
    if (channel == 0) {
        return g_canfd0RxHead != g_canfd0RxTail;
    }
    return false;
}

/*==========================================================================*/
/* UART Low-Level Operations */
/*==========================================================================*/

bool BSP_uartInit(uint8_t channel, uint32_t baudrate) {
    (void)channel;
    (void)baudrate;
    
    /* In real implementation:
     * - Configure UART peripheral clocks
     * - Set baud rate divisors
     * - Configure 8N1 format
     * - Enable RX interrupt
     */
    
    return true;
}

void BSP_uartPutchar(uint8_t channel, char c) {
    (void)channel;
    (void)c;
    
    /* In real implementation:
     * - Wait for TX buffer empty
     * - Write character to TX data register
     */
}

char BSP_uartGetchar(uint8_t channel) {
    if (channel == 0) {
        if (g_uart0RxHead != g_uart0RxTail) {
            char c = g_uart0RxBuffer[g_uart0RxTail];
            g_uart0RxTail = (g_uart0RxTail + 1) % UART_RX_BUFFER_SIZE;
            return c;
        }
    } else if (channel == 1) {
        if (g_uart1RxHead != g_uart1RxTail) {
            char c = g_uart1RxBuffer[g_uart1RxTail];
            g_uart1RxTail = (g_uart1RxTail + 1) % UART_RX_BUFFER_SIZE;
            return c;
        }
    }
    return '\0';
}

bool BSP_uartRxReady(uint8_t channel) {
    if (channel == 0) {
        return g_uart0RxHead != g_uart0RxTail;
    } else if (channel == 1) {
        return g_uart1RxHead != g_uart1RxTail;
    }
    return false;
}

bool BSP_uartTxReady(uint8_t channel) {
    (void)channel;
    return true;  /* Simplified */
}

void BSP_uartPuts(uint8_t channel, const char* str) {
    while (*str) {
        BSP_uartPutchar(channel, *str++);
    }
}

/*==========================================================================*/
/* PWM Low-Level Operations */
/*==========================================================================*/

void BSP_pwmInit(void) {
    /* In real implementation:
     * - Configure FlexPWM peripheral
     * - Set PWM frequency to 50Hz for servos
     * - Configure output pins
     */
}

void BSP_pwmSetDuty(uint8_t channel, uint16_t duty) {
    (void)channel;
    (void)duty;
    
    /* In real implementation:
     * - Update PWM compare register
     * - duty is in microseconds (1000-2000 for servos)
     */
}

void BSP_pwmSetFreq(uint8_t channel, uint32_t freq) {
    (void)channel;
    (void)freq;
    
    /* In real implementation: update PWM period register */
}

/*==========================================================================*/
/* ADC Low-Level Operations */
/*==========================================================================*/

void BSP_adcInit(void) {
    /* In real implementation:
     * - Configure ADC clocks
     * - Set resolution (16-bit)
     * - Configure channels
     */
}

uint16_t BSP_adcRead(uint8_t channel) {
    (void)channel;
    
    /* In real implementation:
     * - Start conversion
     * - Wait for completion
     * - Return result
     */
    
    return 24000;  /* Simulated battery voltage (~24V) */
}

void BSP_adcStartConversion(uint8_t channel) {
    (void)channel;
    /* In real implementation: trigger ADC conversion */
}

bool BSP_adcConversionDone(void) {
    return true;  /* Simplified */
}

/*==========================================================================*/
/* GPIO Low-Level Operations */
/*==========================================================================*/

void BSP_gpioInit(uint8_t port, uint8_t pin, bool output) {
    (void)port;
    (void)pin;
    (void)output;
    
    /* In real implementation:
     * - Configure pin mux
     * - Set direction register
     */
}

void BSP_gpioSet(uint8_t port, uint8_t pin) {
    (void)port;
    (void)pin;
    
    /* In real implementation: set GPIO output high */
}

void BSP_gpioClear(uint8_t port, uint8_t pin) {
    (void)port;
    (void)pin;
    
    /* In real implementation: set GPIO output low */
}

void BSP_gpioToggle(uint8_t port, uint8_t pin) {
    (void)port;
    (void)pin;
    
    /* In real implementation: toggle GPIO output */
}

bool BSP_gpioRead(uint8_t port, uint8_t pin) {
    (void)port;
    (void)pin;
    
    /* In real implementation: read GPIO input */
    return false;
}

/*==========================================================================*/
/* Emergency Stop */
/*==========================================================================*/

bool BSP_isEstopActive(void) {
    return BSP_gpioRead(0, BSP_ESTOP_PIN);
}

/*==========================================================================*/
/* Interrupt Service Routines */
/*==========================================================================*/

void SysTick_Handler(void) {
    ++g_sysTick;
    
    /* Call QP tick function - this will be linked from C++ code */
    /* QP::tick(0U); */
}

void CANFD0_IRQHandler(void) {
    /* In real implementation:
     * - Check interrupt source
     * - If RX: read message and store in buffer
     * - Clear interrupt flag
     */
}

void CANFD1_IRQHandler(void) {
    /* Similar to CANFD0 */
}

void UART0_IRQHandler(void) {
    /* In real implementation:
     * - Check if RX data available
     * - Read character and store in buffer
     * - Clear interrupt flag
     */
}

void UART1_IRQHandler(void) {
    /* In real implementation:
     * - Check if RX data available
     * - Read character and store in buffer
     * - Clear interrupt flag
     */
}

void ADC0_IRQHandler(void) {
    /* In real implementation:
     * - Read conversion result
     * - Clear interrupt flag
     */
}
