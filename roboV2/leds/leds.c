#include <stdint.h>              // Tipos de datos enteros de tamaño fijo (uint32_t, int16_t, etc.)
#include <stdbool.h>             // Tipo booleano (bool) con valores true y false
#include "inc/hw_memmap.h"       // Mapa de memoria del hardware, define las direcciones base de los periféricos (GPIO, UART, etc.)
#include "driverlib/sysctl.h"    // Funciones de control del sistema (configuración de reloj, habilitación de periféricos)
#include "driverlib/gpio.h"      // Control de los pines GPIO (configuración de entrada/salida, lectura y escritura de pines)
#include "driverlib/pin_map.h"   // Asignación de funciones a los pines (mapeo de pines para UART, PWM, etc.)
#include "driverlib/timer.h"     // Funciones para controlar los temporizadores (configuración, inicio, interrupciones)
#include "driverlib/interrupt.h" // Manejo de interrupciones (habilitar/deshabilitar interrupciones, configurar manejadores)
#include "inc/hw_ints.h"         // Definición de las interrupciones del hardware (vectores de interrupción)
#include "driverlib/pwm.h"       // Control del módulo PWM (configuración, generación de señales PWM)
#include "driverlib/uart.h"      // Control del módulo UART (envío y recepción de datos seriales)
#include "utils/uartstdio.h"     // Funciones auxiliares para la entrada/salida por UART (UARTprintf, UARTgets)
#include "driverlib/adc.h"       // Funciones para controlar el ADC (configuración, inicio de conversiones)
#include <string.h>

//
#include "driverlib/systick.h"

#define duty 4095 // ciclo de trabajo maximo del pwm en este caso debe de ser el tamaño de la resolucion de bist del adc 12bits
uint32_t ui32SysClock; // Variable para almacenar la frecuencia del reloj del sistema
uint32_t width;        // Variable para almacenar el ancho del pulso de PWM
uint32_t adcValue;
static void configureSysClock(void); // Configurar el sistema a 120 MHz
static void configureUART0(void);    // Configurar UART0
static void InitPWM1(void);           // Inicializar PWM 1
static void InitPWM2(void);           // Inicializar PWM 2
static void config_adc(void);        // Inicializa el ADC
static void config_gpio(void);      //Configuracion GPIOs
void Timer0IntHandler(void);
void UART0IntHandler(void);

uint32_t ReadADC(void);               //lectura del adc
//--------------------------MAIN----------------------//
int main(void) {
    configureSysClock(); // Configurar el sistema a 120 MHz
    configureUART0();    // Configurar UART0
    InitPWM1();           // Inicializar PWM 1
    InitPWM2();           // Inicializar PWM 2
    config_adc();        // Inicializa el ADC
    config_gpio();       // inicializar GPIO
    width = 0;          //porcentaje de pwm segun la variable duty
    while (1) {
        adcValue = ReadADC(); // Llamar a la función para leer el ADC
        width=adcValue;
        UARTprintf("ADC Value: %d\r\n", adcValue); // Enviar el valor del ADC
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, width);
        GPIOPinWrite(GPIO_PORTN_BASE,GPIO_PIN_2, GPIO_PIN_2); //pin 2
        GPIOPinWrite(GPIO_PORTN_BASE,0x3C, 0x3C); //pin 2,3,4,5
        if(adcValue <= 10)
        {
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 5);
        }
        SysCtlDelay(ui32SysClock*0.2); // Ajustar el valor de delay según sea necesario

    }
}

// Leer valor del ADC
uint32_t ReadADC(void) {
    // Disparar el ADC
    ADCProcessorTrigger(ADC0_BASE, 3);

    // Esperar a que la conversión esté completa
    while (!ADCIntStatus(ADC0_BASE, 3, false)) {}

    // Leer el valor del ADC
    ADCSequenceDataGet(ADC0_BASE, 3, &adcValue);

    // Limpiar la bandera de interrupción
    ADCIntClear(ADC0_BASE, 3);

    return adcValue; // Retornar el valor leído del ADC
}

// Inicializar ADC
void config_adc(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Habilitar el reloj para el puerto GPIO E (donde está PE3)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // Habilitar el reloj para el módulo ADC0

    // Configurar el pin PE3 como entrada analógica para el ADC
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3); // Configurar el pin como entrada de ADC

    // Esperar a que el módulo ADC esté listo
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}

    // Configurar el secuenciador 3 para el ADC
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0); // Configurar secuenciador 3
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END); // Configurar paso 0
    ADCSequenceEnable(ADC0_BASE, 3); // Habilitar el secuenciador 3
    ADCIntClear(ADC0_BASE, 3); // Limpiar interrupciones del secuenciador 3
}

// Configurar UART0
static void configureUART0(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Habilitar el reloj para GPIO A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);  // Habilitar el reloj para UART0

    // Configurar los pines para UART0
    GPIOPinConfigure(GPIO_PA0_U0RX); // Configurar PA0 como RX
    GPIOPinConfigure(GPIO_PA1_U0TX); // Configurar PA1 como TX
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1); // Configurar como UART

    // Configurar UART0 a 9600 baudios, 8 bits de datos, sin paridad, 1 bit de parada
    UARTConfigSetExpClk(UART0_BASE, ui32SysClock, 115200,
                        (UART_CONFIG_WLEN_8 |
                         UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));
    
    // Inicializar la UART estándar para printf
    UARTStdioConfig(0, 115200, ui32SysClock);
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

    // Configurar el generador PWM0
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, duty); // Establecer el período del PWM (frecuencia de 250 Hz)

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

    // Configurar el generador PWM1
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, duty); // Establecer el período del PWM

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, 0); // Establecer el ciclo de trabajo inicial
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)) { }
    PWMGenEnable(PWM0_BASE, PWM_GEN_1); // Habilitar el generador PWM
    PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true); // Habilitar la salida PWM
}


static void config_gpio(void){
    //todo los perifericos que tienen gpio
    //habilitacion de perifericos
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); 
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
    //
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x3F); //0,1,2,3,4,5

}

void Timer0IntHandler(void){

}
void UART0IntHandler(void){

}

// Configurar el reloj del sistema
static void configureSysClock(void) {
    // Configurar el sistema para usar el PLL y establecer la frecuencia a 120 MHz
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_PLL |
                                       SYSCTL_CFG_VCO_240),
                                       120000000);
}