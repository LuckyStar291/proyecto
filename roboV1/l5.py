import gpiod
import time
import serial

# Configuración del HC-SR04 (BCM/GPIO)
TRIG_GPIO = 23  # GPIO23 (pin físico 16)
ECHO_GPIO = 24  # GPIO24 (pin físico 18)

# Configuración UART (RPi → TIVA)
UART_PORT = '/dev/ttyACM0'  # Puerto serial (GPIO14-TX, GPIO15-RX)
BAUD_RATE = 9600          # Baudios (debe coincidir con la TIVA)

# Inicializar GPIO del sensor
chip = gpiod.Chip('gpiochip0')
trig = chip.get_line(TRIG_GPIO)
echo = chip.get_line(ECHO_GPIO)

trig.request(consumer="TRIG", type=gpiod.LINE_REQ_DIR_OUT)
echo.request(consumer="ECHO", type=gpiod.LINE_REQ_DIR_IN)

# Inicializar UART
ser = serial.Serial(UART_PORT, BAUD_RATE, timeout=1)

def medir_distancia():
    trig.set_value(0)
    time.sleep(0.05)
    
    # Pulso de 10μs en TRIG
    trig.set_value(1)
    time.sleep(0.00001)
    trig.set_value(0)

    # Esperar flanco de subida (ECHO)
    timeout = time.time() + 0.1
    while echo.get_value() == 0:
        if time.time() > timeout:
            return -1
        inicio = time.time()
    
    # Esperar flanco de bajada (ECHO)
    timeout = time.time() + 0.1
    while echo.get_value() == 1:
        if time.time() > timeout:
            return -1
        fin = time.time()
    
    distancia = (fin - inicio) * 17150  # Distancia en cm
    return distancia

try:
    while True:
        dist = medir_distancia()
        if dist != -1:
            print(f"Distancia: {dist:.2f} cm")
            # Enviar distancia a la TIVA (formato simple: "DIST:XX.XX")
            ser.write(f"DIST:{dist:.2f}\n".encode('ascii'))
        else:
            print("No se pudo medir")
        time.sleep(0.2)  # Evita saturar el UART

except KeyboardInterrupt:
    print("\nPrograma detenido")

finally:
    # Liberar recursos
    trig.release()
    echo.release()
    chip.close()
    ser.close()