#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>  // Para verificar caracteres numéricos
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
#include "driverlib/pwm.h"

#include "inc/hw_nvic.h"
#include "utils/uartstdio.h"


uint32_t ui32SysClock;
uint8_t counter = 0;
volatile uint8_t ledState = 0;
char data[50]; // Buffer para almacenar los datos recibidos por UART
#define duty 4095 // ciclo de trabajo maximo del pwm en este caso debe de ser el tamaño de la resolucion de bist del adc 12bits
uint32_t PWMCycle1;
uint32_t PWMCycle2;

// Prototipos
static void configureSysClock(void);
static void configGPIO(void);
static void configUART(void);
static void configInterrupt(void);
void ProcesarCadena(void);
void ButtonISR(void);
static void InitPWM1(void);
static void InitPWM2(void);



int main(void) {
    configureSysClock();
    configGPIO();
    configUART();
    InitPWM1();
    InitPWM2();
    configInterrupt(); // Configurar interrupción

    UARTprintf("\n\nUART Listo\r\n");

    while (1) {
        ProcesarCadena();
        
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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // UART

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}
    // Configurar User LEDs (PORTN, PIN_0 y PIN_1)
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x03); //user led PN 0 1
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x3C); // PN 2,3,4,5

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

void ProcesarCadena(void) {
    if (UARTCharsAvail(UART0_BASE)) {
        UARTgets(data, sizeof(data)); // Leer los datos recibidos
        UARTprintf("data recibida: %s\n", data);
        char *token;
        char *tokens[6];
        uint32_t valor = 0;
        int8_t pwmA = 0, pwmB = 0;
        int i = 0;

        // Separar la cadena por comas
        token = strtok(data, ",");
        while (token != NULL && i < 6) {
            tokens[i++] = token;
            token = strtok(NULL, ",");
        }

        // Validar si hay exactamente 6 valores
        if (i == 6) {
            // Validar los primeros 4 valores (deben ser '0' o '1')
            for (i = 0; i < 4; i++) {
                if (tokens[i][0] != '0' && tokens[i][0] != '1') {
                    goto error; // Si no es válido, ir a error
                }
                valor |= (tokens[i][0] - '0') << (3 - i); // Mapear bits en uint32_t
            }

            pwmA = atoi(tokens[4]);
            pwmB = atoi(tokens[5]);

            UARTprintf("pwmA y B: %d, %d \n", pwmA, pwmB);
            

            // Escribir en GPIOs (PN2, PN3, PN4, PN5)
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, (valor & 0x08) ? GPIO_PIN_2 : 0);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, (valor & 0x04) ? GPIO_PIN_3 : 0);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_4, (valor & 0x02) ? GPIO_PIN_4 : 0);
            GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_5, (valor & 0x01) ? GPIO_PIN_5 : 0);

            // Configurar PWM con el duty cycle recibido
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWMCycle1 * pwmA*0.01);
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, PWMCycle2 * pwmB*0.01);
            
        } 
        else {
            error: // Si falla la validación
            strcat(data, " desde Tiva\n"); // Agregar mensaje de error
            UARTprintf(data);
        }
    }
}


// Inicializar PWM
static void InitPWM1(void) {
    /*
- **BASE:** `PWM0_BASE` (Usa el módulo PWM0)
- **GEN:** `PWM_GEN_0` (Usa el generador 0 dentro de PWM0)
- **PIN:** `PF1` (Usa el pin PF1 como salida de PWM)
- **PWM Output:** `PWM_OUT_1` (Salida PWM asociada a PF1)
*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilitar el reloj para GPIO F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);  // Habilitar el reloj para el módulo PWM0

    // Configurar PF1 como salida de PWM
    GPIOPinConfigure(GPIO_PF1_M0PWM1);  // PF1 -> Módulo 0 PWM1
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1); // Configurar PF1 como PWM

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
    uint32_t ui32PWMClockRate = ui32SysClock / 8;
    PWMCycle1 = ui32PWMClockRate / 250;

    // Configurar el generador PWM0
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWMCycle1); // Establecer el período del PWM (frecuencia de 250 Hz)

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 0); // Establecer el ciclo de trabajo inicial
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) { }
    PWMGenEnable(PWM0_BASE, PWM_GEN_0); // Habilitar el generador PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true); // Habilitar la salida PWM
}

// Inicializar segundo PWM
static void InitPWM2(void) {
    /*
- **BASE:** `PWM0_BASE` (Usa el módulo PWM0)
- **GEN:** `PWM_GEN_1` (Usa el generador 1 dentro de PWM0)
- **PIN:** `PF2` (Usa el pin PF2 como salida de PWM)
- **PWM Output:** `PWM_OUT_2` (Salida PWM asociada a PF2)
*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Habilitar el reloj para GPIO F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);  // Habilitar el reloj para el módulo PWM0

    // Configurar PF2 como salida de PWM
    GPIOPinConfigure(GPIO_PF2_M0PWM2);  // PF2 -> Módulo 0 PWM2
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2); // Configurar PF2 como PWM

    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
    uint32_t ui32PWMClockRate = ui32SysClock / 8;
    PWMCycle2 = ui32PWMClockRate / 250;

    // Configurar el generador PWM1
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, PWMCycle2); // Establecer el período del PWM

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, 0); // Establecer el ciclo de trabajo inicial
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) { }
    PWMGenEnable(PWM0_BASE, PWM_GEN_1); // Habilitar el generador PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true); // Habilitar la salida PWM
}