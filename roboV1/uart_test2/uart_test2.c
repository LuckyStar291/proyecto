#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#include "inc/hw_nvic.h"
#include "utils/uartstdio.h"


uint32_t ui32SysClock;
uint8_t counter = 0;
volatile uint8_t ledState = 0;
char data[100]; // Buffer para almacenar los datos recibidos por UART


// Prototipos
static void configureSysClock(void);
static void configGPIO(void);
static void configUART(void);
static void configInterrupt(void);
void ButtonISR(void);


int main(void) {
    configureSysClock();
    configGPIO();
    configUART();
    configInterrupt(); // Configurar interrupción

    UARTprintf("UART Listo\r\n");

    while (1) {
        // Verificar si hay caracteres disponibles en UART0
        if (UARTCharsAvail(UART0_BASE)) {
            UARTgets(data, 100); // Leer los datos recibidos
            
            // Comparar el mensaje recibido y encender los LEDs según corresponda
            if (strcmp(data, "led1") == 0) {
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, GPIO_PIN_0); // Enciende LED1 (PN0)
                UARTprintf("LED1 encendido\n");
            } 
            else if (strcmp(data, "led2") == 0) {
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, GPIO_PIN_1); // Enciende LED2 (PN1)
                UARTprintf("LED2 encendido\n");
            } 
            else if (strcmp(data, "off1") == 0) {
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0); // Apaga LED1
                UARTprintf("LED1 apagado\n");
            } 
            else if (strcmp(data, "off2") == 0) {
                GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0); // Apaga LED2
                UARTprintf("LED2 apagado\n");
            } 
            else if (strcmp(data, "buzzer") == 0) {
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3); // encender buzzer
                UARTprintf("BUZZER\n");
                SysCtlDelay(120000000 / 3 * 2); // Aproximadamente 1 segundo
                GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x00); // encender buzzer

            } 
            else {
                strcat(data, " desde Tiva\n"); // Agregar mensaje al final del texto recibido
                UARTprintf(data); // Enviar el mensaje de vuelta por UART
            }
        }

        // Verificar si se presiona el botón de usuario 1 (PJ0)
        if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0) {
            UARTprintf("motor1\n"); // Enviar mensaje "motor1"
            SysCtlDelay(12000000); // Pequeña pausa para evitar rebotes
        }

        // Verificar si se presiona el botón de usuario 2 (PJ1)
        if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == 0) {
            UARTprintf("motor2\n"); // Enviar mensaje "motor2"
            SysCtlDelay(12000000); // Pequeña pausa para evitar rebotes
        }
        
    }
}

// ===============================
//      CONFIGURACIONES
// ===============================

static void configureSysClock(void) {
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_240), 120000000);
}


static void configGPIO(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Pulsadores
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); // LEDs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // UART

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}

    // Configurar User LEDs (PORTN, PIN_0 y PIN_1)
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x03);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, 0x08); //pin 3

    // Configurar User Switch (PORTJ, pin 0 y 1
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, 0x03);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, 0x03, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // Pull-up interno
}

static void configUART(void) {
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,
                        (UART_CONFIG_WLEN_8 |
                         UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

    UARTEnable(UART0_BASE);
    UARTStdioConfig(0, 115200, ui32SysClock);
}
// ===============================
//    CONFIGURACIÓN DE INTERRUPCIÓN
// ===============================
static void configInterrupt(void) {
    GPIOIntDisable(GPIO_PORTJ_BASE, GPIO_PIN_0); // Deshabilitar interrupciones mientras se configuran
    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0); // Limpiar interrupciones previas
    GPIOIntRegister(GPIO_PORTJ_BASE, ButtonISR); // Registrar la ISR (función de interrupción)
    GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_FALLING_EDGE); // Configurar para detectar flanco de bajada
    GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_PIN_0); // Habilitar interrupción en el pin
    MAP_IntEnable(INT_GPIOJ); // Habilitar interrupción global en PORTJ
    MAP_IntMasterEnable(); // Habilitar interrupciones globales
}

// ===============================
//    FUNCIÓN DE INTERRUPCIÓN
// ===============================

void ButtonISR(void) {
    GPIOIntClear(GPIO_PORTJ_BASE, GPIO_PIN_0); // Limpiar bandera de interrupción

    // Alternar LEDs
    ledState = !ledState;
    if (ledState) {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_0); // Enciende LED1
    } else {
        GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_1); // Enciende LED2
    }

    UARTprintf("Botón presionado. LED cambiado.\r\n");
}