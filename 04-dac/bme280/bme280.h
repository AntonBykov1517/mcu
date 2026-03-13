#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C адреса BME280
#define BME280_I2C_ADDR_0 0x76
#define BME280_I2C_ADDR_1 0x77

// Адреса регистров BME280
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_DATA         0xF7
#define BME280_REG_CALIB        0x88
#define BME280_REG_CALIB_HUM    0xE1

// Ожидаемый ID чипа
#define BME280_CHIP_ID          0x60

// Режимы работы
#define BME280_MODE_SLEEP       0x00
#define BME280_MODE_FORCED      0x01
#define BME280_MODE_NORMAL      0x03

// Коэффициенты oversampling
#define BME280_OSRS_OFF         0x00
#define BME280_OSRS_1           0x01
#define BME280_OSRS_2           0x02
#define BME280_OSRS_4           0x03
#define BME280_OSRS_8           0x04
#define BME280_OSRS_16          0x05

// Время ожидания в нормальном режиме
#define BME280_T_SB_0_5         0x00  // 0.5 ms
#define BME280_T_SB_62_5        0x01  // 62.5 ms
#define BME280_T_SB_125         0x02  // 125 ms
#define BME280_T_SB_250         0x03  // 250 ms
#define BME280_T_SB_500         0x04  // 500 ms
#define BME280_T_SB_1000        0x05  // 1000 ms
#define BME280_T_SB_2000        0x06  // 2000 ms
#define BME280_T_SB_4000        0x07  // 4000 ms

// Коэффициенты фильтра
#define BME280_FILTER_OFF       0x00
#define BME280_FILTER_2         0x01
#define BME280_FILTER_4         0x02
#define BME280_FILTER_8         0x03
#define BME280_FILTER_16        0x04

// Структура для калибровочных коэффициентов
typedef struct {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
    
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
} bme280_calib_data_t;

// Структура для сырых данных
typedef struct {
    int32_t temperature;
    uint32_t pressure;
    uint32_t humidity;
} bme280_raw_data_t;

// Структура для компенсированных данных
typedef struct {
    float temperature;  // °C
    float pressure;     // hPa
    float humidity;     // %
} bme280_data_t;

// Функции инициализации и конфигурации
bool bme280_init(i2c_inst_t *i2c, uint pin_sda, uint pin_scl);
bool bme280_read_id(uint8_t *id);
void bme280_configure(uint8_t mode, uint8_t osrs_temp, uint8_t osrs_press, 
                      uint8_t osrs_hum, uint8_t t_sb, uint8_t filter);

// Функции чтения данных
bool bme280_read_raw(bme280_raw_data_t *raw);
bool bme280_read_all(bme280_data_t *data);

// Компенсационные функции
float bme280_compensate_temperature(int32_t raw_temp);
float bme280_compensate_pressure(int32_t raw_press);
float bme280_compensate_humidity(int32_t raw_hum);