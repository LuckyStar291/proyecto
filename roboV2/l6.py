#!/usr/bin/env python3
import RPi.GPIO as GPIO
import time

# Configuración
MOTOR_PWM_PIN = 18        # GPIO18 para PWM (físico pin 12)
MOTOR_IN1_PIN = 16        # Control dirección 1
MOTOR_IN2_PIN = 20        # Control dirección 2
PWM_FREQ = 1000           # Frecuencia PWM en Hz (1kHz)

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(MOTOR_PWM_PIN, GPIO.OUT)
    GPIO.setup(MOTOR_IN1_PIN, GPIO.OUT)
    GPIO.setup(MOTOR_IN2_PIN, GPIO.OUT)
    
    pwm = GPIO.PWM(MOTOR_PWM_PIN, PWM_FREQ)
    pwm.start(0)  # Inicia con motor detenido
    
    # Configurar dirección hacia adelante
    GPIO.output(MOTOR_IN1_PIN, GPIO.HIGH)
    GPIO.output(MOTOR_IN2_PIN, GPIO.LOW)
    
    return pwm

def test_fixed_duty_cycles(pwm):
    """Prueba ciclos de trabajo fijos"""
    print("\n--- Prueba de ciclos fijos ---")
    duty_cycles = [75, 25, 45, 50]  # Ciclos a probar
    
    for duty in duty_cycles:
        print(f"Motor a {duty}% de potencia")
        pwm.ChangeDutyCycle(duty)
        time.sleep(3)  # Mantener cada ciclo por 3 segundos
    
    pwm.ChangeDutyCycle(0)  # Detener motor
    print("Prueba completada\n")
    time.sleep(1)

def acceleration_sequence(pwm):
    """Secuencia de aceleración progresiva"""
    print("\n--- Iniciando secuencia de aceleración ---")
    print("Presiona Ctrl+C para detener")
    
    try:
        while True:
            # Aceleración progresiva (0% a 100%)
            for duty in range(0, 101):
                pwm.ChangeDutyCycle(duty)
                print(f"Duty Cycle: {duty}%", end='\r')
                time.sleep(0.5)  # Paso cada 0.5 segundos
            
            # Reinicio suave
            for duty in range(100, -1, -1):
                pwm.ChangeDutyCycle(duty)
                print(f"Duty Cycle: {duty}%", end='\r')
                time.sleep(0.1)
                
            time.sleep(1)  # Pausa antes de repetir
            
    except KeyboardInterrupt:
        pwm.ChangeDutyCycle(0)
        print("\nSecuencia detenida")

def main():
    pwm = setup()
    
    try:
        # 1. Prueba de ciclos fijos
        test_fixed_duty_cycles(pwm)
        
        # 2. Secuencia de aceleración
        acceleration_sequence(pwm)
        
    except KeyboardInterrupt:
        print("\nPrograma interrumpido por el usuario")
    finally:
        pwm.stop()
        GPIO.cleanup()
        print("GPIO liberados")

if __name__ == "__main__":
    main()