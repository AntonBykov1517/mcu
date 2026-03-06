#pragma once

typedef enum
{
    ADC_TASK_STATE_IDLE = 0,
    ADC_TASK_STATE_RUN = 1,
} adc_task_state_t;

void adc_task_init();
float adc_task_read();
float adc_task_read_temperature();
void adc_task_handle();              // новый прототип
void adc_task_set_state(adc_task_state_t state);  // новый прототип