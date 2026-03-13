#include "bme280.h"
#include <stdio.h>

// Статические переменные
static i2c_inst_t *i2c_port;
static uint8_t i2c_addr;
static bme280_calib_data_t calib;
static int32_t t_fine;

// Поиск адреса BME280
static bool find_device_address() {
    uint8_t chip_id;
    
    // Пробуем первый адрес
    i2c_addr = BME280_I2C_ADDR_0;
    uint8_t reg = BME280_REG_ID;
    i2c_write_blocking(i2c_port, i2c_addr, &reg, 1, true);
    if (i2c_read_blocking(i2c_port, i2c_addr, &chip_id, 1, false) >= 0) {
        if (chip_id == BME280_CHIP_ID) {
            return true;
        }
    }
    
    // Пробуем второй адрес
    i2c_addr = BME280_I2C_ADDR_1;
    i2c_write_blocking(i2c_port, i2c_addr, &reg, 1, true);
    if (i2c_read_blocking(i2c_port, i2c_addr, &chip_id, 1, false) >= 0) {
        if (chip_id == BME280_CHIP_ID) {
            return true;
        }
    }
    
    return false;
}

// Чтение регистра по I2C
static uint8_t read_register(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c_port, i2c_addr, &reg, 1, true);
    i2c_read_blocking(i2c_port, i2c_addr, &value, 1, false);
    return value;
}

// Запись в регистр по I2C
static void write_register(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    i2c_write_blocking(i2c_port, i2c_addr, data, 2, false);
}

// Чтение нескольких байт
static void read_registers(uint8_t reg, uint8_t *buffer, size_t len) {
    i2c_write_blocking(i2c_port, i2c_addr, &reg, 1, true);
    i2c_read_blocking(i2c_port, i2c_addr, buffer, len, false);
}

// Чтение калибровочных коэффициентов
static void read_calibration_data() {
    uint8_t buffer[32];
    
    // Чтение калибровки температуры и давления (0x88 - 0xA1)
    read_registers(0x88, buffer, 26);
    
    // Распаковка калибровки температуры
    calib.dig_T1 = (uint16_t)(buffer[1] << 8 | buffer[0]);
    calib.dig_T2 = (int16_t)(buffer[3] << 8 | buffer[2]);
    calib.dig_T3 = (int16_t)(buffer[5] << 8 | buffer[4]);
    
    // Распаковка калибровки давления
    calib.dig_P1 = (uint16_t)(buffer[7] << 8 | buffer[6]);
    calib.dig_P2 = (int16_t)(buffer[9] << 8 | buffer[8]);
    calib.dig_P3 = (int16_t)(buffer[11] << 8 | buffer[10]);
    calib.dig_P4 = (int16_t)(buffer[13] << 8 | buffer[12]);
    calib.dig_P5 = (int16_t)(buffer[15] << 8 | buffer[14]);
    calib.dig_P6 = (int16_t)(buffer[17] << 8 | buffer[16]);
    calib.dig_P7 = (int16_t)(buffer[19] << 8 | buffer[18]);
    calib.dig_P8 = (int16_t)(buffer[21] << 8 | buffer[20]);
    calib.dig_P9 = (int16_t)(buffer[23] << 8 | buffer[22]);
    
    // Чтение калибровки влажности (0xE1 - 0xE7)
    read_registers(0xE1, buffer, 8);
    
    calib.dig_H1 = buffer[0];
    calib.dig_H2 = (int16_t)(buffer[2] << 8 | buffer[1]);
    calib.dig_H3 = buffer[3];
    calib.dig_H4 = (int16_t)((buffer[4] << 4) | (buffer[5] & 0x0F));
    calib.dig_H5 = (int16_t)((buffer[6] << 4) | ((buffer[5] & 0xF0) >> 4));
    calib.dig_H6 = (int8_t)buffer[7];
    
    printf("BME280 calibration data loaded\n");
}

// Инициализация BME280
bool bme280_init(i2c_inst_t *i2c, uint pin_sda, uint pin_scl) {
    i2c_port = i2c;
    
    // Инициализация I2C
    i2c_init(i2c_port, 100000);  // 100 kHz
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_set_function(pin_scl, GPIO_FUNC_I2C);
    gpio_pull_up(pin_sda);
    gpio_pull_up(pin_scl);
    
    // Поиск устройства
    if (!find_device_address()) {
        printf("BME280 not found!\n");
        return false;
    }
    
    printf("BME280 found at address 0x%02X\n", i2c_addr);
    
    // Чтение калибровочных данных
    read_calibration_data();
    
    return true;
}

// Чтение ID датчика
bool bme280_read_id(uint8_t *id) {
    *id = read_register(BME280_REG_ID);
    return (*id == BME280_CHIP_ID);
}

// Конфигурация режимов работы
void bme280_configure(uint8_t mode, uint8_t osrs_temp, uint8_t osrs_press, 
                      uint8_t osrs_hum, uint8_t t_sb, uint8_t filter) {
    // Настройка влажности
    write_register(BME280_REG_CTRL_HUM, osrs_hum);
    
    // Настройка температуры, давления и режима
    uint8_t ctrl_meas = (osrs_temp << 5) | (osrs_press << 2) | mode;
    write_register(BME280_REG_CTRL_MEAS, ctrl_meas);
    
    // Настройка standby и фильтра
    uint8_t config = (t_sb << 5) | (filter << 2);
    write_register(BME280_REG_CONFIG, config);
    
    printf("BME280 configured\n");
}

// Чтение сырых данных
bool bme280_read_raw(bme280_raw_data_t *raw) {
    uint8_t data[8];
    
    read_registers(BME280_REG_DATA, data, 8);
    
    // Распаковка данных (20 бит на измерение)
    raw->pressure    = (data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
    raw->temperature = (data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
    raw->humidity    = (data[6] << 8) | data[7];
    
    return true;
}

// Компенсация температуры
float bme280_compensate_temperature(int32_t raw_temp) {
    int32_t var1, var2;
    
    var1 = ((((raw_temp >> 3) - ((int32_t)calib.dig_T1 << 1))) * 
            ((int32_t)calib.dig_T2)) >> 11;
    
    var2 = (((((raw_temp >> 4) - ((int32_t)calib.dig_T1)) * 
              ((raw_temp >> 4) - ((int32_t)calib.dig_T1))) >> 12) * 
            ((int32_t)calib.dig_T3)) >> 14;
    
    t_fine = var1 + var2;
    
    return (t_fine * 5 + 128) >> 8;
}

// Компенсация давления
float bme280_compensate_pressure(int32_t raw_press) {
    int64_t var1, var2, p;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + 
           ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    
    return (float)p / 256.0f;
}

// Компенсация влажности
float bme280_compensate_humidity(int32_t raw_hum) {
    int32_t v_x1_u32r;
    
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((raw_hum << 14) - (((int32_t)calib.dig_H4) << 20) - 
                   (((int32_t)calib.dig_H5) * v_x1_u32r)) + 
                   ((int32_t)16384)) >> 15) * 
                   (((((((v_x1_u32r * ((int32_t)calib.dig_H6)) >> 10) * 
                      (((v_x1_u32r * ((int32_t)calib.dig_H3)) >> 11) + 
                      ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
                      ((int32_t)calib.dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                ((int32_t)calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    
    return (float)(v_x1_u32r >> 12) / 1024.0f;
}

// Чтение всех данных
bool bme280_read_all(bme280_data_t *data) {
    bme280_raw_data_t raw;
    
    if (!bme280_read_raw(&raw)) {
        return false;
    }
    
    float temp_raw = bme280_compensate_temperature(raw.temperature);
    data->temperature = temp_raw / 100.0f;
    
    float press_pa = bme280_compensate_pressure(raw.pressure);
    data->pressure = press_pa / 100.0f;
    
    data->humidity = bme280_compensate_humidity(raw.humidity);
    
    return true;
}