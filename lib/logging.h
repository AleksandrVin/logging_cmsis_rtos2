/**
 * @file interface.h
 * @brief Header file for interface.c
 *
 * @date 2024-01
 * @author Aleksandr Vinogradov
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"

#define LOG_BUFFER_SIZE 200

#define LOG_QUEUE_ROWS 10
#define LOG_MUTEX_TIMEOUT 200
#define LOG_SEMAPHORE_TIMEOUT 200
#define LOG_SEMAPHORE_COUNT_MAX (LOG_QUEUE_ROWS)

extern char LOGGING_BUF[LOG_BUFFER_SIZE];
extern char LOGGING_ISR_BUF[LOG_BUFFER_SIZE];

#define INTERFACE_BUFFER_SIZE (200 + 20)
extern char INTERFACE_BUFFER[INTERFACE_BUFFER_SIZE];

#define UART_INTERFACE huart1

// #define INTERFACE_printf(...) printf(__VA_ARGS__)
#define INTERFACE_printf(...)                           \
    HAL_UART_Transmit(&UART_INTERFACE, (uint8_t *)INTERFACE_BUFFER, \
                       snprintf((char *)INTERFACE_BUFFER, \
                                LOG_BUFFER_SIZE, __VA_ARGS__), 100)

typedef enum log_levels
{
    DEBUG_ALL = 0,
    DEBUG_MIN,
    INFO,
    WARNING,
    ERR
} log_levels_t;

#define LOG_TYPE_SIZE 10

// switch logging level. Only level above or equal to this will be printed
#define LOGGING_LEVEL DEBUG_ALL

void logging_log(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);
void log_ISR(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);

#define LOG_FATAL(...) INTERFACE_printf(__VA_ARGS__)

// better to replace this with custom timer implementation for better performance
#define UPTIME_MS (osKernelGetTickCount() * 1000 / osKernelGetTickFreq()) % 1000
#define UPTIME_S osKernelGetTickCount() / osKernelGetTickFreq() 

// on FAILURE log does not work. Use LOG_INTERRUPT instead as native printf
#define LOG(level, ...)                                                   \
    if (level >= LOGGING_LEVEL)                                           \
    {                                                                     \
        snprintf(LOGGING_BUF, LOG_BUFFER_SIZE, __VA_ARGS__);        \
        logging_log(LOGGING_BUF, UPTIME_S, UPTIME_MS, level); \
    }

#define LOG_ISR(level, ...)                                               \
    if (level >= LOGGING_LEVEL)                                           \
    {                                                                     \
        snprintf(LOGGING_ISR_BUF, LOG_BUFFER_SIZE, __VA_ARGS__);    \
        log_ISR(LOGGING_ISR_BUF, UPTIME_S, UPTIME_MS, level); \
    }

void logging_init();
void logging_send_to_interface();

void print_swo(const char *data, const uint32_t size);

#endif // LOGGING_H