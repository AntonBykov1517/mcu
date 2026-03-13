#include "bme280-driver.h"
#include "bme280-regs.h"

typedef struct
{
    bme280_i2c_read i2c_read;
    bme280_i2c_write i2c_write;
    
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
    
    int32_t t_fine;
} bme280_ctx_t;

static bme280_ctx_t bme280_ctx = {0};

static void bme280_read_calibration();

void bme280_init(bme280_i2c_read i2c_read, bme280_i2c_write i2c_write)
{
    bme280_ctx.i2c_read = i2c_read;
    bme280_ctx.i2c_write = i2c_write;
    
    uint8_t id_reg_buf[1] = {0};
    bme280_read_regs(BME280_REG_id, id_reg_buf, sizeof(id_reg_buf));
    
    uint8_t ctrl_hum_reg_value = 0;
    ctrl_hum_reg_value |= (0b001 << 0);
    bme280_write_reg(BME280_REG_ctrl_hum, ctrl_hum_reg_value);
    
    uint8_t config_reg_value = 0;
    config_reg_value |= (0b0 << 0);     
    config_reg_value |= (0b000 << 2);   
    config_reg_value |= (0b001 << 5);   
    bme280_write_reg(BME280_REG_config, config_reg_value);
    
    uint8_t ctrl_meas_reg_value = 0;
    ctrl_meas_reg_value |= (0b001 << 5); 
    ctrl_meas_reg_value |= (0b001 << 2); 
    ctrl_meas_reg_value |= (0b11 << 0);  
    bme280_write_reg(BME280_REG_ctrl_meas, ctrl_meas_reg_value);
    
    bme280_read_calibration();
}

void bme280_read_regs(uint8_t start_reg_address, uint8_t* buffer, uint8_t length)
{
    uint8_t data[1] = {start_reg_address};
    bme280_ctx.i2c_write(data, sizeof(data));
    bme280_ctx.i2c_read(buffer, length);
}

void bme280_write_reg(uint8_t reg_address, uint8_t value)
{
    uint8_t data[2] = {reg_address, value};
    bme280_ctx.i2c_write(data, sizeof(data));
}

static void bme280_read_calibration()
{
    uint8_t buffer[32];
    
    bme280_read_regs(0x88, buffer, 26);
    
    bme280_ctx.dig_T1 = (uint16_t)(buffer[1] << 8 | buffer[0]);
    bme280_ctx.dig_T2 = (int16_t)(buffer[3] << 8 | buffer[2]);
    bme280_ctx.dig_T3 = (int16_t)(buffer[5] << 8 | buffer[4]);
    
    bme280_ctx.dig_P1 = (uint16_t)(buffer[7] << 8 | buffer[6]);
    bme280_ctx.dig_P2 = (int16_t)(buffer[9] << 8 | buffer[8]);
    bme280_ctx.dig_P3 = (int16_t)(buffer[11] << 8 | buffer[10]);
    bme280_ctx.dig_P4 = (int16_t)(buffer[13] << 8 | buffer[12]);
    bme280_ctx.dig_P5 = (int16_t)(buffer[15] << 8 | buffer[14]);
    bme280_ctx.dig_P6 = (int16_t)(buffer[17] << 8 | buffer[16]);
    bme280_ctx.dig_P7 = (int16_t)(buffer[19] << 8 | buffer[18]);
    bme280_ctx.dig_P8 = (int16_t)(buffer[21] << 8 | buffer[20]);
    bme280_ctx.dig_P9 = (int16_t)(buffer[23] << 8 | buffer[22]);
    
    bme280_read_regs(0xE1, buffer, 8);
    
    bme280_ctx.dig_H1 = buffer[0];
    bme280_ctx.dig_H2 = (int16_t)(buffer[2] << 8 | buffer[1]);
    bme280_ctx.dig_H3 = buffer[3];
    bme280_ctx.dig_H4 = (int16_t)((buffer[4] << 4) | (buffer[5] & 0x0F));
    bme280_ctx.dig_H5 = (int16_t)((buffer[6] << 4) | ((buffer[5] >> 4) & 0x0F));
    bme280_ctx.dig_H6 = (int8_t)buffer[7];
}

uint16_t bme280_read_temp_raw()
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_temp_lsb, read, sizeof(read));
    uint16_t value = ((uint16_t)read[1] << 8) | ((uint16_t)read[0]);
    return value;
}

uint16_t bme280_read_press_raw()
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_press_lsb, read, sizeof(read));
    uint16_t value = ((uint16_t)read[1] << 8) | ((uint16_t)read[0]);
    return value;
}

uint16_t bme280_read_hum_raw()
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_hum_lsb, read, sizeof(read));
    uint16_t value = ((uint16_t)read[1] << 8) | ((uint16_t)read[0]);
    return value;
}

float bme280_read_temperature()
{
    int32_t var1, var2, t;
    
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_temp_msb, read, 3);
    int32_t adc_T = ((int32_t)read[0] << 12) | ((int32_t)read[1] << 4) | (read[2] >> 4);
    
    var1 = ((((adc_T >> 3) - ((int32_t)bme280_ctx.dig_T1 << 1))) * 
            ((int32_t)bme280_ctx.dig_T2)) >> 11;
    
    var2 = (((((adc_T >> 4) - ((int32_t)bme280_ctx.dig_T1)) * 
              ((adc_T >> 4) - ((int32_t)bme280_ctx.dig_T1))) >> 12) * 
            ((int32_t)bme280_ctx.dig_T3)) >> 14;
    
    bme280_ctx.t_fine = var1 + var2;
    t = (bme280_ctx.t_fine * 5 + 128) >> 8;
    
    return (float)t / 100.0f;
}

float bme280_read_pressure()
{
    uint8_t read[3] = {0};
    bme280_read_regs(BME280_REG_press_msb, read, 3);
    int32_t adc_P = ((int32_t)read[0] << 12) | ((int32_t)read[1] << 4) | (read[2] >> 4);
    
    int64_t var1, var2, p;
    
    var1 = ((int64_t)bme280_ctx.t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)bme280_ctx.dig_P6;
    var2 = var2 + ((var1 * (int64_t)bme280_ctx.dig_P5) << 17);
    var2 = var2 + (((int64_t)bme280_ctx.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)bme280_ctx.dig_P3) >> 8) + 
           ((var1 * (int64_t)bme280_ctx.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)bme280_ctx.dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)bme280_ctx.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)bme280_ctx.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)bme280_ctx.dig_P7) << 4);
    
    return (float)p / 25600.0f;
}

float bme280_read_humidity()
{
    uint8_t read[2] = {0};
    bme280_read_regs(BME280_REG_hum_msb, read, 2);
    int32_t adc_H = ((int32_t)read[0] << 8) | read[1];
    
    int32_t v_x1_u32r;
    
    v_x1_u32r = (bme280_ctx.t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme280_ctx.dig_H4) << 20) - 
                   (((int32_t)bme280_ctx.dig_H5) * v_x1_u32r)) + 
                   ((int32_t)16384)) >> 15) * 
                   (((((((v_x1_u32r * ((int32_t)bme280_ctx.dig_H6)) >> 10) * 
                      (((v_x1_u32r * ((int32_t)bme280_ctx.dig_H3)) >> 11) + 
                      ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * 
                      ((int32_t)bme280_ctx.dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * 
                ((int32_t)bme280_ctx.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    
    return (float)(v_x1_u32r >> 12) / 1024.0f;
}