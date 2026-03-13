import time
import serial
import matplotlib.pyplot as plt
from datetime import datetime

def read_value(ser):
    """Читает строку из порта и преобразует в число"""
    while True:
        try:
            line = ser.readline().decode('ascii').strip()
            if line:
                return float(line)
        except:
            continue

def main():
    ser = serial.Serial(port='COM9', baudrate=115200, timeout=1.0)
    
    if not ser.is_open:
        print("Port not opened")
        return
    
    print(f"Port {ser.name} opened")
    
    # Списки для хранения данных
    timestamps = []
    temperatures = []
    pressures = []
    humidities = []
    
    start_time = time.time()

    try:
        while True:
            # Текущее время
            t = time.time() - start_time
            
            # Запрос температуры
            ser.write("temp\n".encode('ascii'))
            temp = read_value(ser)
            
            # Запрос давления
            ser.write("pres\n".encode('ascii'))
            pres = read_value(ser)
            
            # Запрос влажности
            ser.write("hum\n".encode('ascii'))
            hum = read_value(ser)
            
            # Сохраняем данные
            timestamps.append(t)
            temperatures.append(temp)
            pressures.append(pres)
            humidities.append(hum)
            
            # Выводим в консоль
            print(f"t={t:.1f}s | T={temp:.2f}°C | P={pres:.2f}hPa | H={hum:.1f}%")
            
            # Пауза 1 секунда между измерениями
            time.sleep(1)
            
    except KeyboardInterrupt:
        print("\nСбор данных остановлен")
    finally:
        ser.close()
        print("Порт закрыт")
        
        # Построение графиков
        plt.figure(figsize=(12, 10))
        
        # График температуры
        plt.subplot(3, 1, 1)
        plt.plot(timestamps, temperatures, 'r-', linewidth=2, label='Температура')
        plt.axhline(y=temperatures[0], color='gray', linestyle='--', alpha=0.5, label='Начальное значение')
        plt.ylabel('Температура (°C)')
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        # График давления
        plt.subplot(3, 1, 2)
        plt.plot(timestamps, pressures, 'b-', linewidth=2, label='Давление')
        plt.axhline(y=pressures[0], color='gray', linestyle='--', alpha=0.5, label='Начальное значение')
        plt.ylabel('Давление (гПа)')
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        # График влажности
        plt.subplot(3, 1, 3)
        plt.plot(timestamps, humidities, 'g-', linewidth=2, label='Влажность')
        plt.axhline(y=humidities[0], color='gray', linestyle='--', alpha=0.5, label='Начальное значение')
        plt.xlabel('Время (секунды)')
        plt.ylabel('Влажность (%)')
        plt.legend()
        plt.grid(True, alpha=0.3)
        
        plt.suptitle('Измерения BME280 при дыхании на датчик', fontsize=14)
        plt.tight_layout()
        plt.show()

if __name__ == "__main__":
    main()