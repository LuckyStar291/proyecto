#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include <stdio.h>

#include "string.h"

//#include "driverlib/uart.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.c"
#include "utils/uartstdio.h"

#include "driverlib/pwm.h"
#include "driverlib/adc.h"
#include "driverlib/rom.h"

#include "driverlib/pin_map.h"
char buffer[20]; 
uint32_t FS = 120000000/24;
float distance =0;
uint8_t perdido = 0;
#define SPEED_OF_SOUND_CM_PER_US 0.0343
void timer0A_handler(void);

void int_to_str(int num, char* str) {
    int i = 0;
    int temp = num;
    if (num < 0) {
        *str++ = '-';
        num *= -1;
        temp *= -1;
    }
    while (temp) {
        temp /= 10;
        i++;
    }
    str[i] = '\0';
    while (num) {
        str[--i] = num % 10 + '0';
        num /= 10;
    }
    if (i == 0 && str[0] != '-') {
        str--;
    }
}
void float_to_str(float num, char* str, int precision) {
    int wholePart = (int)num; // Parte entera del número
    float fractionalPart = num - wholePart; // Parte fraccionaria del número
    
    // Convertir la parte entera a una cadena de caracteres
    int_to_str(wholePart, str);
    
    // Apuntar al final de la cadena para agregar la parte fraccionaria
    while (*str != '\0') {
        str++;
    }
    
    // Agregar el punto decimal
    *str++ = '.';
    
    // Convertir la parte fraccionaria a una cadena de caracteres con la precisión especificada
    for (int i = 0; i < precision; i++) {
        fractionalPart *= 10;
        int digit = (int)fractionalPart;
        *str++ = digit + '0';
        fractionalPart -= digit;
    }
    
    // Agregar el carácter nulo al final de la cadena
    *str = '\0';
}
//    #define GPIO_PIN_0              0x00000001  // GPIO pin 0
//#define GPIO_PIN_1              0x00000002  // GPIO pin 1
//#define GPIO_PIN_2              0x00000004  // GPIO pin 2
//#define GPIO_PIN_3              0x00000008  // GPIO pin 3
//#define GPIO_PIN_4              0x00000010  // GPIO pin 4
//#define GPIO_PIN_5              0x00000020  // GPIO pin 5
//#define GPIO_PIN_6              0x00000040  // GPIO pin 6
//#define GPIO_PIN_7              0x00000080  // GPIO pin 7



int main(void) {
//conf
//uart> PA1 PA0
//timer  timer 0A
//gpio led 1-4 PN1 PN0 PF4 PF0
//PWM PG1
//ADC PK3
//HC sensor echo PM5  trigger  PM4
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),120000000);
    
    //periph uart
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    //gpio
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }
    
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x03);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, 0x11);
    GPIOPinTypeGPIOOutput(GPIO_PORTM_BASE, 0x10);

    
    GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, 0x20);
    GPIOPadConfigSet(GPIO_PORTM_BASE, 0x20, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

    //timer
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    // set the cout time for the Timer
    TimerLoadSet(TIMER0_BASE, TIMER_A, FS);
    // enable processor interrupts
    IntMasterEnable();
    // enable interrupt
    IntEnable(INT_TIMER0A);
    // enable timer A interrupt
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // enable the timer
    TimerEnable(TIMER0_BASE, TIMER_A);

    //uart
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, 0X03);
    UARTStdioConfig(0, 9600, 120000000);
    UARTEnable(UART0_BASE);
    
    //PWM
    // Habilitar el módulo PWM0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    // Habilitar el puerto GPIO G para PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
    GPIOPinConfigure(GPIO_PG1_M0PWM5); // Puerto G, Pin 1 es PWM5
    GPIOPinTypePWM(GPIO_PORTG_BASE, GPIO_PIN_1); // Configurar el pin como PWM

    // Configurar el generador PWM en el módulo PWM0, generador 2
    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, SysCtlClockGet() / 261); 
    //PWMGenPeriodSet(PWM0_BASE, PWM_GEN_2, 1000); Frecuencia del PWM: 1 kHz (1000 ciclos)

    // Configurar el ciclo de trabajo inicial del PWM (50%)
    PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT, true); // Habilitar la salida PWM
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, 10); // Ciclo de trabajo del 1%

    // Habilitar el generador PWM
    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
    
    
    
    // ADC
    //Por defecto, el divisor de reloj del ADC se establece en 64, lo que significa que el ADC se muestrea a una frecuencia de reloj de SYSCLK/64
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    GPIOPinTypeADC(GPIO_PORTK_BASE, 0x08); // Configurar el pin como adc
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, 10);//stablece el divisor de reloj del ADC en 10, lo que significa que la frecuencia de muestreo será SYSCLK/10
    ADCSequenceConfigure(ADC0_BASE,3,ADC_TRIGGER_PROCESSOR,0);
    ADCSequenceStepConfigure(ADC0_BASE,3,0,ADC_CTL_IE|ADC_CTL_END|ADC_CTL_CH19);
    ADCSequenceEnable(ADC0_BASE,3);
    
    
        // Configurar Timer1 como temporizador de un solo disparo descendente
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {}
    TimerConfigure(TIMER1_BASE, TIMER_CFG_ONE_SHOT); // Cuenta regresiva (modo DOWN)

    
    //uint32_t ui32Value=10;
    //int valor=1;

        while (1) {
        memset(buffer, 0, sizeof(buffer));
        uint32_t startVal = 0;
        uint32_t endVal = 0;
        uint32_t elapsed = 0;
        uint32_t clockFreq = SysCtlClockGet(); // Obtenemos frecuencia actual (debería ser 120 MHz)
        float time_us = 0;

        // Trigger del sensor ultrasónico
        GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4, GPIO_PIN_4);
        SysCtlDelay(clockFreq / (1000000 / 10));  // Espera de 10 us
        GPIOPinWrite(GPIO_PORTM_BASE, GPIO_PIN_4, 0);

        // Esperar flanco de subida
        uint32_t timeout = 1000000;
        while (!GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_5) && timeout--) {}
        if (timeout == 0) {
            distance = 0;
            UARTprintf("Echo no detectado\n");
            SysCtlDelay(clockFreq / 10); // Esperar 100ms
            continue;
        }

        // Iniciar temporizador desde un valor alto
        TimerLoadSet(TIMER1_BASE, TIMER_A, 0xFFFFFF);
        TimerEnable(TIMER1_BASE, TIMER_A);

        // Esperar flanco de bajada
        while (GPIOPinRead(GPIO_PORTM_BASE, GPIO_PIN_5)) {}
        endVal = TimerValueGet(TIMER1_BASE, TIMER_A);
        TimerDisable(TIMER1_BASE, TIMER_A);

        // Calcular tiempo transcurrido
        elapsed = 0xFFFFFF - endVal;
        time_us = ((float)elapsed * 1000000.0f) / (float)clockFreq;

        // Calcular distancia en cm
        distance = (time_us * SPEED_OF_SOUND_CM_PER_US) / 60.0f;

        if (distance > 400.0f) distance = 0; // Rango máximo válido

        float_to_str(distance, buffer, 2);
        UARTprintf("Distancia: %s cm\n", buffer);

        SysCtlDelay(clockFreq / 1); // 100 ms entre mediciones
    }

}

void timer0A_handler(void)
{
//gpio led 1-4 PN1 PN0 PF4 PF0

	if (distance>10){
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x0);
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
  
        }
	else if (distance>=8 && distance<10){
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00000002);
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x0);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
  
        }
        else if (distance>=6 && distance<8){
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00000002);
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00000001);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x0);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
  
        }
        else if (distance>=4 && distance<6){
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00000002);
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00000001);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00000010);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x0);
  
        }
        else if (distance<4){
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0x00000002);
        	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0x00000001);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_4, 0x00000010);
        	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, 0x00000001);
  
        }
        
	TimerIntClear(TIMER0_BASE, TIMER_A);
     
        
	
}
