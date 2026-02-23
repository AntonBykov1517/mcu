#include <stdio.h>
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"
#include "led-task/led-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

// Прототипы функций
void version_callback(const char* args);
void led_on_callback(const char* args);
void led_off_callback(const char* args);
void led_blink_callback(const char* args);
void led_blink_set_period_ms_callback(const char* args);
void help_callback(const char* args);
void mem_callback(const char* args);      // новая
void wmem_callback(const char* args);     // новая

// Device API commands array
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
    {NULL, NULL, NULL},
};

// Existing callbacks
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

// Callback for setting blink period
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

// Help callback
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

// NEW: mem callback - read memory at hex address
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

// NEW: wmem callback - write memory at hex address
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

int main() {
    stdio_init_all();
    
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    
    while (1) {
        char* command = stdio_task_handle();
        protocol_task_handle(command);
        led_task_handle();
    }
    return 0;
}