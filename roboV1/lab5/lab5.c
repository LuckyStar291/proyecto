#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "inc/hw_ints.h"

#include "string.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.c"
#include "utils/uartstdio.h"

#include "driverlib/pwm.h"
#include "driverlib/rom.h"

#include "driverlib/pin_map.h"

void timer0A_handler(void);

char msg1[10]="motor1\n";
char msg2[10]="motor2\n";
char data[10] = "";
char msg[10] = "";

void delay(int num){
	volatile uint32_t ui32Loop;
	for(ui32Loop = 0; ui32Loop < num; ui32Loop++){
        	
        	UARTgets(data, 10);
        	strcpy(msg, data);
		strcat(msg, "\n");
        	
    	
	
        }
}
int main(void) {
    
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),120000000);
    
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION))
    {
    }
    
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, 0X03);
    
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);        
    
    
    UARTStdioConfig(0, 9600, 120000000);
    
    
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
    
    
    
    
    //while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)){}
    //GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x02);
    //GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, 0x02);
    memset(msg, 0, sizeof(msg));
   
    while (1) {
	    UARTEnable(UART0_BASE);
	   
	    if (UARTCharsAvail(UART0_BASE) == 0) {
	    	UARTprintf(msg);
	    	
	    }
	    else{
	    	UARTgets(data, 10);
        	strcpy(msg, data);
		strcat(msg, "\n");
	    }
	    if (strcmp(msg, "1\n") == 0) {
    			GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x02);
    			//for (int duty = 0; duty <= 1000; duty += 10) {
			 //   PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, duty);
			  //  SysCtlDelay(SysCtlClockGet() / 100); //
        		//}
        		PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, SysCtlClockGet() / 522);
        		
    		
		}
		else{
    			GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x00);
    			 PWMPulseWidthSet(PWM0_BASE, PWM_OUT_5, 1);
    			
		}

	   SysCtlDelay((120000000 / (5)));
        	
    	//if (data[0] != ' ') {
    		//memset(msg, 0, sizeof(msg));

		//memset(data, 0, sizeof(data));
			
    	//}
		
	//SysCtlDelay((120000000 / (3))*2);
    	//GPIOPinWrite(GPIO_PORTN_BASE, 0x01, 0x01);
    	//SysCtlDelay(1000000);
    	//strcat(data, "\n");
    	//UARTprintf(data[0]);
    	//int i=0;
    	//while (UARTCharsAvail(UART0_BASE)){
    		//data[i] = UARTCharGet(UART0_BASE);
    		//i++;
    	//}

	//GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x00);
    	
    	//GPIOPinWrite(GPIO_PORTN_BASE, 0x02, 0x00);
    	//SysCtlDelay((120000000 / (3))*2);
    	//
    	//strcat(data, "\n");
    	//UARTprintf(data);
	    	
    	
    }
}

void timer0A_handler(void)
{

}
