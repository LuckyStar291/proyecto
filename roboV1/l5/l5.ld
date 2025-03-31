/******************************************************************************
 * Linker Script para EK-TM4C1294XL - Versión Corregida
 * Soluciona: 
 * - Superposición .ARM.exidx/.data
 * - Gestión explícita de secciones ARM
 * - Optimización de espacio en FLASH/SRAM
 *****************************************************************************/

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00100000  /* 1MB Flash */
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x00040000  /* 256KB SRAM */
}

SECTIONS
{
    /* --- Sección de código y constantes en FLASH --- */
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))    /* Vectores de interrupción */
        *(.text*)               /* Código */
        *(.rodata*)             /* Datos de solo lectura */
        . = ALIGN(4);           /* Alineación a 4 bytes */
        _etext = .;
    } > FLASH

    /* --- Sección para manejo de excepciones ARM (CRITICO) --- */
    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* --- Datos inicializados (copia desde FLASH a SRAM) --- */
    .data : AT (ADDR(.text) + SIZEOF(.text) + SIZEOF(.ARM.exidx)) /* ¡Ajuste clave! */
    {
        _data = .;
        _ldata = LOADADDR (.data);
        *(vtable)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

    /* --- Datos NO inicializados (SRAM) --- */
    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* --- Heap y Stack (opcional) --- */
    .heap :
    {
        _heap_start = .;
        . = . + 0x8000;         /* 32KB Heap (ajustar según necesidad) */
        _heap_end = .;
    } > SRAM

    .stack :
    {
        _stack_end = .;
        . = . + 0x4000;         /* 16KB Stack (ajustar) */
        _stack_top = .;
    } > SRAM

    /* --- Símbolos para depuración --- */
    _total_flash = SIZEOF(.text) + SIZEOF(.ARM.exidx) + SIZEOF(.data);
    _total_sram = SIZEOF(.data) + SIZEOF(.bss) + 0x8000 + 0x4000;
}