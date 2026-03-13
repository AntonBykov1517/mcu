#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "hardware/i2c.h"
#include "bme280-driver.h"

void read_regs_callback(const char* args);

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void help_callback(const char* args);
void mem_callback(const char* args);     
void wmem_callback(const char* args);     
void write_reg_callback(const char* args);
void temp_raw_callback(const char* args);
void press_raw_callback(const char* args);
void hum_raw_callback(const char* args);
void temp_callback(const char* args);
void pres_callback(const char* args);
void hum_callback(const char* args);

api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turn LED on"},
    {"off", led_off_callback, "turn LED off"},
    {"blink", led_blink_callback, "make LED blink"},
    {"set_period", led_blink_set_period_ms_callback, "set blink period in milliseconds"},
    {"mem", mem_callback, "read memory: mem <hex_address>"},
    {"wmem", wmem_callback, "write memory: wmem <hex_address> <hex_value>"},
    {"help", help_callback, "show this help message"},
    {"write_reg", write_reg_callback, "write BME280 register: write_reg <addr> <value>"},
    {"read_regs", read_regs_callback, "read BME280 registers: read_regs <addr> <count>"},
    {"temp_raw", temp_raw_callback, "read raw temperature value from BME280"},
    {"press_raw", press_raw_callback, "read raw pressure value from BME280"},
    {"temp", temp_callback, "read temperature in °C"},
    {"pres", pres_callback, "read pressure in hPa"},
    {"hum", hum_callback, "read humidity in %"},
    {"hum_raw", hum_raw_callback, "read raw humidity value from BME280"},
    {NULL, NULL, NULL},
};

void version_callback(const char* args)
{
    printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args)
{
    led_task_state_set(LED_STATE_ON);
    printf("LED turned ON\n");
}

void led_off_callback(const char* args)
{
    led_task_state_set(LED_STATE_OFF);
    printf("LED turned OFF\n");
}

void led_blink_callback(const char* args)
{
    led_task_state_set(LED_STATE_BLINK);
    printf("LED blinking started\n");
}

void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);
    
    if (period_ms == 0)
    {
        printf("Error: invalid period value. Usage: set_period <milliseconds>\n");
        return;
    }
    
    led_task_set_blink_period_ms(period_ms);
    printf("LED blink period set to %u ms\n", period_ms);
}

void help_callback(const char* args)
{
    printf("\nAvailable commands:\n");
    printf("------------------\n");
    
    for (int i = 0; device_api[i].command_name != NULL; i++)
    {
        printf("%-12s - %s\n", device_api[i].command_name, device_api[i].command_help);
    }
    printf("\n");
}

void mem_callback(const char* args)
{
    uint32_t addr = 0;
    sscanf(args, "%x", &addr);
    
    if (addr == 0)
    {
        printf("Error: invalid address. Usage: mem <hex_address>\n");
        return;
    }
    
    uint32_t value = *(uint32_t*)addr;
    printf("mem[0x%08X] = 0x%08X (%u)\n", addr, value, value);
}

void wmem_callback(const char* args)
{
    uint32_t addr = 0;
    uint32_t value = 0;
    sscanf(args, "%x %x", &addr, &value);
    
    if (addr == 0)
    {
        printf("Error: invalid arguments. Usage: wmem <hex_address> <hex_value>\n");
        return;
    }
    
    *(uint32_t*)addr = value;
    printf("wmem[0x%08X] = 0x%08X\n", addr, value);
}

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
    i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
    i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

void read_regs_callback(const char* args)
{
    uint32_t addr = 0, count = 0;
    sscanf(args, "%x %x", &addr, &count);
    
    if (addr > 0xFF || count > 0xFF || addr + count > 0x100) {
        printf("Error: invalid address or count\n");
        return;
    }
    
    uint8_t buffer[256] = {0};
    bme280_read_regs(addr, buffer, count);
    
    for (int i = 0; i < count; i++) {
        printf("bme280 register [0x%X] = 0x%X\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char* args)
    {
        uint32_t addr = 0, value = 0;
        sscanf(args, "%x %x", &addr, &value);
        
        if (addr > 0xFF || value > 0xFF) {
            printf("Error: address and value must be 0-0xFF\n");
            return;
        }
        
        bme280_write_reg(addr, value);
        printf("bme280 register [0x%X] written with 0x%X\n", addr, value);
    }
void temp_raw_callback(const char* args)
{
    uint16_t value = bme280_read_temp_raw();
    printf("temp_raw = %u (0x%X)\n", value, value);
}

void press_raw_callback(const char* args)
{
    uint16_t value = bme280_read_press_raw();
    printf("press_raw = %u (0x%X)\n", value, value);
}

void hum_raw_callback(const char* args)
{
    uint16_t value = bme280_read_hum_raw();
    printf("hum_raw = %u (0x%X)\n", value, value);
}

void temp_callback(const char* args)
{
    float temp = bme280_read_temperature();
    printf("%.2f\n", temp);
}

void pres_callback(const char* args)
{
    float pres = bme280_read_pressure();
    printf("%.2f\n", pres);
}

void hum_callback(const char* args)
{
    float hum = bme280_read_humidity();
    printf("%.2f\n", hum);
}

int main() {
    stdio_init_all();
    
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);

    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    printf("BME280 driver initialized\n");
    
    while (1) {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();
    }
    return 0;
}