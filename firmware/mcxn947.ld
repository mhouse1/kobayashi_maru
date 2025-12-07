/* Linker script for FRDM-MCXN947 (MCXN947)
 * Cortex-M33 with 2MB Flash, 512KB SRAM
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 2048K
    RAM (rwx)   : ORIGIN = 0x20000000, LENGTH = 512K
}

ENTRY(Reset_Handler)

/* Stack top at end of RAM */
PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        PROVIDE(_sdata = .);
        *(.data)
        *(.data*)
        . = ALIGN(4);
        PROVIDE(_edata = .);
    } > RAM AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        PROVIDE(_sbss = .);
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        PROVIDE(_ebss = .);
    } > RAM

    .heap :
    {
        . = ALIGN(8);
        _heap_start = .;
        . = . + 0x1000;  /* 4KB heap */
        _heap_end = .;
    } > RAM
}
