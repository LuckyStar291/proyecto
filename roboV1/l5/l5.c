
 #include <stdint.h>
 #include <stdbool.h>
 #include <string.h>
 #include "inc/hw_memmap.h"
 #include "driverlib/debug.h"
 #include "driverlib/gpio.h"
 #include "driverlib/sysctl.h"
 #include "driverlib/pin_map.h"
 #include "driverlib/uart.h"
 #include "utils/uartstdio.h"
 //#include "utils/uartstdio.c"
 
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"
#include "driverlib/pwm.h"
#include "driverlib/rom.h"

//
#include "driverlib/systick.h"
#include <stdlib.h>

 char data[100]; // Buffer para almacenar los datos recibidos por UART
 char msg1[10]="motor1\n";
char msg2[10]="motor2\n";

 int main(void)
 {
     // Configurar la frecuencia del sistema a 120 MHz
     SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
     
     // Habilitar los periféricos UART0, GPIOA, GPIOJ y GPION (para LEDs de usuario)
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Para los botones
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION); // Para los LEDs de usuario
 
     // Configurar los pines PA0 y PA1 para comunicación UART0
     GPIOPinConfigure(GPIO_PA0_U0RX);
     GPIOPinConfigure(GPIO_PA1_U0TX);
     GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
 
     // Configurar botones de usuario en PJ0 y PJ1 como entradas con pull-up
     GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
     GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
 
     // Configurar los LEDs de usuario (PN0 y PN1) como salida
     GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0 | GPIO_PIN_1);
 
     // Configurar UART0 con una velocidad de 9600 baudios
     UARTStdioConfig(0, 9600, 120000000);
     
     while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
     {
     }
    
     GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x00000003);
    
     GPIOPinConfigure(GPIO_PA0_U0RX);
     GPIOPinConfigure(GPIO_PA1_U0TX);
     GPIOPinTypeUART(GPIO_PORTA_BASE, 0X03);
     
     GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
     GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
     GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
     GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);        
     
     
     UARTStdioConfig(0, 9600, 120000000);
     
     //while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)){}
     //GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x02);
     //GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x02);
     
     
     while (1)
     {
        UARTEnable(UART0_BASE);
    	
    
    
    	//SysCtlDelay((120000000 / (3))*2);
    	
        if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_0) == 0) {    	
    		GPIOPinWrite(GPIO_PORTN_BASE, 0x01, 0x01);
    	
    		UARTprintf(msg1);
    		SysCtlDelay((120000000 / (3))*2);
    	
    	}
    	if (GPIOPinRead(GPIO_PORTJ_BASE, GPIO_PIN_1) == 0) {    	
    		GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x02);
    	
    		UARTprintf(msg2);
    	
    		SysCtlDelay((120000000 / (3))*2);
    	}
    	
    	GPIOPinWrite(GPIO_PORTN_BASE, 0x01, 0x00);
    	GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x00);

     }
 }