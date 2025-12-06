/*
 * Board Support Package Header
 * FRDM-MCXN947 Freedom Board
 * 
 * Hardware abstraction layer for the robot firmware.
 */

#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>

/*==========================================================================*/
/* System Configuration */
/*==========================================================================*/

#define BSP_TICKS_PER_SEC       1000U
#define BSP_CPU_FREQ            150000000U  /* 150 MHz */

/*==========================================================================*/
/* CAN-FD Configuration */
/*==========================================================================*/

#define BSP_CANFD0_BASE         0x4009C000U
#define BSP_CANFD1_BASE         0x4009D000U
#define BSP_CANFD_BITRATE       5000000U    /* 5 Mbps */

/*==========================================================================*/
/* UART Configuration */
/*==========================================================================*/

#define BSP_UART0_BASE          0x40106000U  /* Debug console */
#define BSP_UART1_BASE          0x40107000U  /* Android/Pixel 10 Pro */
#define BSP_UART_BAUDRATE       115200U

/*==========================================================================*/
/* PWM Configuration for Turret Servos */
/*==========================================================================*/

#define BSP_PWM0_BASE           0x40088000U
#define BSP_PWM_FREQ            50U          /* 50 Hz for servos */

/*==========================================================================*/
/* GPIO Configuration */
/*==========================================================================*/

#define BSP_GPIO0_BASE          0x40096000U
#define BSP_GPIO1_BASE          0x40098000U

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
/* BSP Function Prototypes */
/*==========================================================================*/

/* Initialization */
void BSP_init(void);
void BSP_start(void);

/* System */
uint32_t BSP_get_tick(void);
void BSP_delay_ms(uint32_t ms);

/* LED control */
void BSP_led_on(uint8_t led);
void BSP_led_off(uint8_t led);
void BSP_led_toggle(uint8_t led);

/* CAN-FD operations */
bool BSP_canfd_init(uint8_t channel, uint32_t bitrate);
bool BSP_canfd_send(uint8_t channel, uint32_t id, const uint8_t *data, uint8_t dlc);
bool BSP_canfd_receive(uint8_t channel, uint32_t *id, uint8_t *data, uint8_t *dlc);
void BSP_canfd_set_filter(uint8_t channel, uint32_t id, uint32_t mask);

/* UART operations */
bool BSP_uart_init(uint8_t channel, uint32_t baudrate);
void BSP_uart_putchar(uint8_t channel, char c);
char BSP_uart_getchar(uint8_t channel);
bool BSP_uart_rx_ready(uint8_t channel);
void BSP_uart_puts(uint8_t channel, const char *str);

/* PWM operations for servo control */
void BSP_pwm_init(void);
void BSP_pwm_set_duty(uint8_t channel, uint16_t duty);

/* ADC operations for battery monitoring */
uint16_t BSP_adc_read(uint8_t channel);

/* GPIO operations */
void BSP_gpio_set(uint8_t port, uint8_t pin);
void BSP_gpio_clear(uint8_t port, uint8_t pin);
bool BSP_gpio_read(uint8_t port, uint8_t pin);

/* Emergency stop */
bool BSP_is_estop_active(void);

/*==========================================================================*/
/* Debug/Trace */
/*==========================================================================*/

#ifdef DEBUG
    #define BSP_DEBUG_PRINT(msg)    BSP_uart_puts(0, msg)
#else
    #define BSP_DEBUG_PRINT(msg)    ((void)0)
#endif

#endif /* BSP_H */
