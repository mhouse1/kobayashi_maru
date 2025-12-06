/*
 * BSP Low-Level Drivers (C)
 * FRDM-MCXN947 Freedom Board
 * 
 * Low-level hardware drivers, ISRs, and performance-critical routines.
 * This file is pure C for maximum performance and direct hardware access.
 */

#ifndef BSP_DRIVERS_H
#define BSP_DRIVERS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*==========================================================================*/
/* System Configuration */
/*==========================================================================*/

#define BSP_TICKS_PER_SEC       1000U
#define BSP_CPU_FREQ            150000000U  /* 150 MHz */

/*==========================================================================*/
/* Hardware Base Addresses */
/*==========================================================================*/

#define BSP_CANFD0_BASE         0x4009C000U
#define BSP_CANFD1_BASE         0x4009D000U
#define BSP_UART0_BASE          0x40106000U
#define BSP_UART1_BASE          0x40107000U
#define BSP_PWM0_BASE           0x40088000U
#define BSP_GPIO0_BASE          0x40096000U
#define BSP_GPIO1_BASE          0x40098000U
#define BSP_ADC0_BASE           0x400A0000U
#define BSP_TIMER0_BASE         0x40034000U

/*==========================================================================*/
/* Configuration Constants */
/*==========================================================================*/

#define BSP_CANFD_BITRATE       5000000U    /* 5 Mbps */
#define BSP_UART_BAUDRATE       115200U
#define BSP_PWM_FREQ            50U         /* 50 Hz for servos */

/* LED pins */
#define BSP_LED_RED_PIN         0
#define BSP_LED_GREEN_PIN       1
#define BSP_LED_BLUE_PIN        2

/* Button pins */
#define BSP_BTN_SW2_PIN         4
#define BSP_BTN_SW3_PIN         5

/* Emergency stop pin */
#define BSP_ESTOP_PIN           6

/*==========================================================================*/
/* Low-Level Driver Functions (C) */
/*==========================================================================*/

/* System Initialization */
void BSP_init(void);
void BSP_start(void);

/* System Tick (called from SysTick ISR) */
uint32_t BSP_getTick(void);
void BSP_delayMs(uint32_t ms);

/* LED Control (direct GPIO manipulation) */
void BSP_ledOn(uint8_t led);
void BSP_ledOff(uint8_t led);
void BSP_ledToggle(uint8_t led);

/* CAN-FD Low-Level Operations */
bool BSP_canfdInit(uint8_t channel, uint32_t bitrate);
bool BSP_canfdSend(uint8_t channel, uint32_t id, const uint8_t* data, uint8_t dlc);
bool BSP_canfdReceive(uint8_t channel, uint32_t* id, uint8_t* data, uint8_t* dlc);
void BSP_canfdSetFilter(uint8_t channel, uint32_t id, uint32_t mask);
bool BSP_canfdTxReady(uint8_t channel);
bool BSP_canfdRxReady(uint8_t channel);

/* UART Low-Level Operations */
bool BSP_uartInit(uint8_t channel, uint32_t baudrate);
void BSP_uartPutchar(uint8_t channel, char c);
char BSP_uartGetchar(uint8_t channel);
bool BSP_uartRxReady(uint8_t channel);
bool BSP_uartTxReady(uint8_t channel);
void BSP_uartPuts(uint8_t channel, const char* str);

/* PWM Low-Level Operations */
void BSP_pwmInit(void);
void BSP_pwmSetDuty(uint8_t channel, uint16_t duty);
void BSP_pwmSetFreq(uint8_t channel, uint32_t freq);

/* ADC Low-Level Operations */
void BSP_adcInit(void);
uint16_t BSP_adcRead(uint8_t channel);
void BSP_adcStartConversion(uint8_t channel);
bool BSP_adcConversionDone(void);

/* GPIO Low-Level Operations */
void BSP_gpioInit(uint8_t port, uint8_t pin, bool output);
void BSP_gpioSet(uint8_t port, uint8_t pin);
void BSP_gpioClear(uint8_t port, uint8_t pin);
void BSP_gpioToggle(uint8_t port, uint8_t pin);
bool BSP_gpioRead(uint8_t port, uint8_t pin);

/* Emergency Stop */
bool BSP_isEstopActive(void);

/*==========================================================================*/
/* Interrupt Service Routines (implemented in bsp_irq.c) */
/*==========================================================================*/

void SysTick_Handler(void);
void CANFD0_IRQHandler(void);
void CANFD1_IRQHandler(void);
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void ADC0_IRQHandler(void);

/*==========================================================================*/
/* Critical Section Macros */
/*==========================================================================*/

#define BSP_ENTER_CRITICAL()    __asm volatile ("cpsid i" ::: "memory")
#define BSP_EXIT_CRITICAL()     __asm volatile ("cpsie i" ::: "memory")

/*==========================================================================*/
/* Debug/Trace (C interface) */
/*==========================================================================*/

#ifdef DEBUG
    #define BSP_DEBUG_PRINT(msg)    BSP_uartPuts(0, msg)
#else
    #define BSP_DEBUG_PRINT(msg)    ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* BSP_DRIVERS_H */
