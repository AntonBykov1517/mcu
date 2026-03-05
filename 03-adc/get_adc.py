import time
import serial
import matplotlib.pyplot as plt

def read_value(ser):
    while True:
        try:
            line = ser.readline().decode('ascii')
            value = float(line)
            return value
        except ValueError:
            continue

def main():
    # Настройка последовательного порта (ЗАМЕНИТЕ COM3 НА ВАШ ПОРТ!)
    ser = serial.Serial(port='COM7', baudrate=115200, timeout=0.0)
    
    if ser.is_open:
        print(f"Port {ser.name} opened")
    else:
        print(f"Port {ser.name} closed")
        return
    
    # Списки для хранения данных
    measure_temperature_C = []
    measure_voltage_V = []
    measure_ts = []
    
    start_ts = time.time()
    
    try:
        while True:
            # Время измерения
            ts = time.time() - start_ts
            
            # Запрос напряжения
            ser.write("get_adc\n".encode('ascii'))
            voltage_V = read_value(ser)
            
            # Запрос температуры
            ser.write("get_temp\n".encode('ascii'))
            temp_C = read_value(ser)
            
            # Сохраняем данные
            measure_ts.append(ts)
            measure_voltage_V.append(voltage_V)
            measure_temperature_C.append(temp_C)
            
            # Выводим в консоль
            print(f'{voltage_V:.3f} V - {temp_C:.1f}C - {ts:.2f}s')
            
            # Задержка
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\nMeasurement stopped by user")
    finally:
        ser.close()
        print("Port closed")
        
        # Построение графиков
        plt.figure(figsize=(10, 8))
        
        plt.subplot(2, 1, 1)
        plt.plot(measure_ts, measure_voltage_V, 'b-')
        plt.title('График зависимости напряжения от времени')
        plt.xlabel('время, с')
        plt.ylabel('напряжение, В')
        plt.grid(True)
        
        plt.subplot(2, 1, 2)
        plt.plot(measure_ts, measure_temperature_C, 'r-')
        plt.title('График зависимости температуры от времени')
        plt.xlabel('время, с')
        plt.ylabel('температура, °C')
        plt.grid(True)
        
        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    main()