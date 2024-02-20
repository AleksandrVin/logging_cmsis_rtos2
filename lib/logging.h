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

#define WAIT_FOR_SET_INIT_COMMAND 1

#define LOG_BUFFER_SIZE 100

#define LOG_QUEUE_ROWS 5
#define LOG_SEMAPHORE_COUNT_MAX (LOG_QUEUE_ROWS)

extern char LOGGING_BUF[LOG_BUFFER_SIZE];
extern char LOGGING_ISR_BUF[LOG_BUFFER_SIZE];

#define INTERFACE_BUFFER_SIZE (LOG_BUFFER_SIZE + 20)
extern char INTERFACE_BUFFER[INTERFACE_BUFFER_SIZE];

// #define USE_USB_LOGGING // define this to use USB logging as an example of logging extension

// #define INTERFACE_printf(...) printf(__VA_ARGS__)
#ifdef LOGGING_USE_USB
#include "logging_usb.h"
#elif defined LOGGING_USE_CUSTOM_INTERFACE
#include "logging_custom_interface.h"
#else
#define UART_INTERFACE huart1
#define INTERFACE_printf(FATAL_FLAG, ...)                                       \
    HAL_UART_Transmit(&UART_INTERFACE, (uint8_t *)INTERFACE_BUFFER, \
                      snprintf((char *)INTERFACE_BUFFER,            \
                               LOG_BUFFER_SIZE, __VA_ARGS__),       \
                      100)
#endif

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
#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL DEBUG_ALL
#endif

#define FATAL_FLAG_SET 1
#define FATAL_FLAG_CLEAR 0

void logging_log(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);
void log_ISR(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);

#define LOG_FATAL(...) INTERFACE_printf(FATAL_FLAG_SET, __VA_ARGS__)

// better to replace this with custom timer implementation for better performance
#define UPTIME_MS (osKernelGetTickCount() * 1000 / osKernelGetTickFreq()) % 1000
#define UPTIME_S osKernelGetTickCount() / osKernelGetTickFreq()

// on FAILURE log does not work. Use LOG_INTERRUPT instead as native printf
#define LOG(level, ...)                                       \
    if (level >= LOGGING_LEVEL)                               \
    {                                                         \
        snprintf(LOGGING_BUF, LOG_BUFFER_SIZE, __VA_ARGS__);  \
        logging_log(LOGGING_BUF, UPTIME_S, UPTIME_MS, level); \
    }

#define LOG_ISR(level, ...)                                      \
    if (level >= LOGGING_LEVEL)                                  \
    {                                                            \
        snprintf(LOGGING_ISR_BUF, LOG_BUFFER_SIZE, __VA_ARGS__); \
        log_ISR(LOGGING_ISR_BUF, UPTIME_S, UPTIME_MS, level);    \
    }

void logging_init();
void logging_send_to_interface();
int  logging_is_initialized();
void logging_set_init();

void print_swo(const char *data, const uint32_t size);

#endif // LOGGING_H