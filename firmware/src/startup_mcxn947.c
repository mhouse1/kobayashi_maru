/* Minimal startup code for FRDM-MCXN947 (Cortex-M33)
 * TODO: Replace with vendor SDK startup file
 */

#include <stdint.h>

// Stack and heap sizes
#define STACK_SIZE 0x2000
#define HEAP_SIZE  0x1000

// Stack top (end of RAM)
extern uint32_t _estack;

// Defined by linker script
extern uint32_t _sdata, _edata, _sidata;
extern uint32_t _sbss, _ebss;

// Main application entry
extern int main(void);

// Default handler
void Default_Handler(void) {
    while(1);
}

// System initialization (weak, can be overridden)
__attribute__((weak)) void SystemInit(void) {
}

// Reset handler
void Reset_Handler(void) {
    // Copy initialized data from flash to RAM
    uint32_t *src = &_sidata;
    uint32_t *dest = &_sdata;
    while (dest < &_edata) {
        *dest++ = *src++;
    }
    
    // Zero-initialize BSS
    dest = &_sbss;
    while (dest < &_ebss) {
        *dest++ = 0;
    }
    
    // Call system initialization
    SystemInit();
    
    // Call main
    main();
    
    // Hang if main returns
    while(1);
}

// Vector table - keep and used attributes prevent linker from discarding
__attribute__((section(".isr_vector"), used))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),     // Initial stack pointer
    Reset_Handler,                   // Reset handler
    Default_Handler,                 // NMI
    Default_Handler,                 // HardFault
    Default_Handler,                 // MemManage
    Default_Handler,                 // BusFault
    Default_Handler,                 // UsageFault
    Default_Handler,                 // SecureFault
    0,                               // Reserved
    0,                               // Reserved
    0,                               // Reserved
    Default_Handler,                 // SVCall
    Default_Handler,                 // DebugMon
    0,                               // Reserved
    Default_Handler,                 // PendSV
    Default_Handler,                 // SysTick
    // External interrupts (add as needed)
};
