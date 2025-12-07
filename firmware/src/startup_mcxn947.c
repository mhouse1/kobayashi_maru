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

// Reset handler - must be visible to linker
__attribute__((used, externally_visible))
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

// Vector table - must be at address 0x0, aligned to 512 bytes for Cortex-M33
__attribute__((section(".isr_vector"), used, aligned(512)))
const void* const g_pfnVectors[] = {
    (const void*)(&_estack),        // Initial stack pointer
    (const void*)Reset_Handler,     // Reset handler
    (const void*)Default_Handler,   // NMI
    (const void*)Default_Handler,   // HardFault
    (const void*)Default_Handler,   // MemManage
    (const void*)Default_Handler,   // BusFault
    (const void*)Default_Handler,   // UsageFault
    (const void*)Default_Handler,   // SecureFault
    0,                               // Reserved
    0,                               // Reserved
    0,                               // Reserved
    (const void*)Default_Handler,   // SVCall
    (const void*)Default_Handler,   // DebugMon
    0,                               // Reserved
    (const void*)Default_Handler,   // PendSV
    (const void*)Default_Handler,   // SysTick
    // External interrupts (add as needed)
};
