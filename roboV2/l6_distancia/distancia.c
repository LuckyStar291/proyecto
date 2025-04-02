//*******************************************************************
// LIBRERÍAS
//*******************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.c"

//*******************************************************************
// DEFINICIONES
//*******************************************************************
#define LED_PN0 GPIO_PIN_0
#define LED_PN1 GPIO_PIN_1
#define LED_PF4 GPIO_PIN_4
#define LED_PF0 GPIO_PIN_0
#define BAUDRATE 9600
#define SYS_CLOCK 120000000 // 120 MHz

//*******************************************************************
// PROTOTIPOS
//*******************************************************************
void UART0_Init(void);
void LED_Init(void);
void ControlarLEDs(float distancia);
float parseDistancia(const char* buffer);

//*******************************************************************
// FUNCIÓN PRINCIPAL
//*******************************************************************
int main(void) {
    SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), SYS_CLOCK);
    GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_PF0 | LED_PF4, 0);
    UART0_Init();
    LED_Init();
    
    char buffer[32];
    float distancia;
    
    while(1) {
        UARTgets(buffer, sizeof(buffer)); // Bloqueante hasta recibir '\n'
        
        distancia = parseDistancia(buffer);
        if(distancia >= 0) { // -1 indica error
            ControlarLEDs(distancia);
        }
    }
}

//*******************************************************************
// PARSEAR DISTANCIA MANUALMENTE (sin sscanf)
//*******************************************************************
float parseDistancia(const char* buffer) {
    // Verificar si empieza con "DIST:"
    if(strncmp(buffer, "DIST:", 5) != 0) {
        return -1.0f; // Formato incorrecto
    }
    
    const char* num_start = buffer + 5; // Puntero al inicio del número
    float valor = 0.0f;
    float decimal = 0.1f;
    bool punto_encontrado = false;
    
    for(int i = 0; num_start[i] != '\0'; i++) {
        if(num_start[i] == '.') {
            punto_encontrado = true;
            continue;
        }
        
        if(num_start[i] >= '0' && num_start[i] <= '9') {
            if(!punto_encontrado) {
                valor = valor * 10 + (num_start[i] - '0');
            } else {
                valor += (num_start[i] - '0') * decimal;
                decimal *= 0.1f;
            }
        } else if(num_start[i] == '\n' || num_start[i] == '\r') {
            break; // Fin de línea
        } else {
            return -1.0f; // Carácter inválido
        }
    }
    
    return valor;
}

//*******************************************************************
// INICIALIZACIÓN UART (usa UARTStdio)
//*******************************************************************
void UART0_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    
    UARTStdioConfig(0, BAUDRATE, SYS_CLOCK);
}

//*******************************************************************
// CONTROL DE LEDs
//*******************************************************************
void ControlarLEDs(float distancia) {
    // Apagar todos primero
    GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_PF0 | LED_PF4, 0);
    
    if(distancia < 4.0f) {
        // Todos encendidos
        GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, LED_PN0 | LED_PN1);
        GPIOPinWrite(GPIO_PORTF_BASE, LED_PF0 | LED_PF4, LED_PF0 | LED_PF4);
    }
    else if(distancia < 6.0f) {
        GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, LED_PN0 | LED_PN1);
        GPIOPinWrite(GPIO_PORTF_BASE, LED_PF4, LED_PF4);
    }
    else if(distancia < 8.0f) {
        GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, LED_PN0 | LED_PN1);
    }
    else if(distancia < 10.0f) {
        GPIOPinWrite(GPIO_PORTN_BASE, LED_PN1, LED_PN1);
    }
    // >10cm: todos apagados
}

//*******************************************************************
// INICIALIZACIÓN LEDs
//*******************************************************************
void LED_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    
    GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, LED_PN0 | LED_PN1);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_PF0 | LED_PF4);
    
    GPIOPinWrite(GPIO_PORTN_BASE, LED_PN0 | LED_PN1, 0);
    GPIOPinWrite(GPIO_PORTF_BASE, LED_PF0 | LED_PF4, 0);
}