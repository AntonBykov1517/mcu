#include "adc-task.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_GPIO_PIN 26
#define ADC_CHANNEL 0
#define TEMP_SENSOR_CHANNEL 4

// Глобальные переменные состояния
static adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
static uint64_t adc_ts = 0;
static uint64_t ADC_TASK_MEAS_PERIOD_US = 100000;  // 100 мс

void adc_task_init()
{
    adc_init();
    adc_gpio_init(ADC_GPIO_PIN);
    adc_set_temp_sensor_enabled(true);
}

void adc_task_set_state(adc_task_state_t state)
{
    adc_state = state;
    if (state == ADC_TASK_STATE_RUN) {
        adc_ts = 0;  // сброс таймера при запуске
    }
}

float adc_task_read()
{
    adc_select_input(ADC_CHANNEL);
    uint16_t voltage_counts = adc_read();
    float voltage_V = (voltage_counts * 3.3f) / 4096.0f;
    return voltage_V;
}

float adc_task_read_temperature()
{
    adc_select_input(TEMP_SENSOR_CHANNEL);
    uint16_t temp_counts = adc_read();
    float temp_V = (temp_counts * 3.3f) / 4096.0f;
    float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;
    return temp_C;
}

void adc_task_handle()
{
    if (adc_state == ADC_TASK_STATE_RUN)
    {
        uint64_t now = time_us_64();
        if (now > adc_ts)
        {
            adc_ts = now + ADC_TASK_MEAS_PERIOD_US;
            
            float voltage_V = adc_task_read();
            float temp_C = adc_task_read_temperature();
            
            printf("%f %f\n", voltage_V, temp_C);
        }
    }
}