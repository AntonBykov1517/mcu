#include "protocol-task.h"
#include <stdio.h>
#include <string.h>

static api_t* api = {0};
static int commands_count = 0;

void protocol_task_init(api_t* device_api)
{
    api = device_api;
    commands_count = 0;
    
    // Подсчитываем количество команд (до NULL)
    while (api[commands_count].command_name != NULL)
    {
        commands_count++;
    }
    
    printf("Protocol task initialized: %d commands available\n", commands_count);
}

void protocol_task_handle(char* command_string)
{
    // Проверка на NULL
    if (!command_string)
    {
        return;
    }
    
    // Делим строку на команду и аргументы
    const char* command_name = command_string;
    const char* command_args = NULL;
    
    char* space_symbol = strchr(command_string, ' ');
    
    if (space_symbol)
    {
        *space_symbol = '\0';
        command_args = space_symbol + 1;
    }
    else
    {
        command_args = "";
    }
    
    // Выводим найденные имя команды и аргументы
    printf("Command: '%s', Args: '%s'\n", command_name, command_args);
    
    // Ищем команду в массиве
    for (int i = 0; i < commands_count; i++)
    {
        if (strcmp(command_name, api[i].command_name) == 0)
        {
            // Нашли команду - вызываем callback
            api[i].command_callback(command_args);
            return;
        }
    }
    
    // Команда не найдена
    printf("Error: unknown command '%s'\n", command_name);
}
