/**
 * @file interface.h
 * @brief Header file for interface.c
 *
 * Methods to print messages to the console[USB_CDC, UART, SWV]
 *
 * @date 2023-06-16
 * @author Aleksandr Vinogradov / Copyright TerraQuantum
 */

#ifndef LOGGING_H
#define LOGGING_H

#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "tim.h"
#include "wake.h"

#define CDC_WAIT_FOR_PC_TO_READ 0

#define USE_USB_CDC 1
#define USE_UART 2
#define USE_SWV 3
#define USE_WAKE 4

// switch interface for protocols here
#define WAKE_INTERFACE USE_USB_CDC
#define INTERFACE USE_USB_CDC
#define PROTOCOL_LOG USE_USB_CDC

#define INTERFACE_BUFFER_SIZE 200
#define INTERFACE_CDC_BUFFER_SIZE INTERFACE_BUFFER_SIZE + 20

extern char CDC_USB_RX_BUF[INTERFACE_CDC_BUFFER_SIZE];
extern char CDC_USB_TX_BUF[INTERFACE_CDC_BUFFER_SIZE];

#define LOG_QUEUE_ROWS 20
#define LOG_MUTEX_TIMEOUT 200
#define LOG_SEMAPHORE_TIMEOUT 200
#define LOG_SEMAPHORE_COUNT_MAX (LOG_QUEUE_ROWS)

extern char LOGGING_BUF[INTERFACE_BUFFER_SIZE];
extern char LOGGING_ISR_BUF[INTERFACE_BUFFER_SIZE];

#if INTERFACE == USE_USB_CDC
#define INTERFACE_printf(...)         \
    while (CDC_IsBusy() == USBD_BUSY && CDC_WAIT_FOR_PC_TO_READ == 1) \
    {                                 \
        osThreadYield();              \
    }                                 \
    CDC_Transmit_FS((uint8_t *)CDC_USB_TX_BUF, snprintf(CDC_USB_TX_BUF, INTERFACE_CDC_BUFFER_SIZE, __VA_ARGS__))
#elif INTERFACE == USE_SWV
#define INTERFACE_printf(...) printf(__VA_ARGS__)
#endif

#define UPTIME_MS htim13.Instance->CNT

typedef enum log_levels
{
    DEBUG_ALL = 0,
    DEBUG_MIN,
    INFO,
    WARNING,
    ERR
} log_levels_t;

#define LOG_TYPE_SIZE 10

// switch logging level here. Only level above or equal to this will be printed
#define LOGGING_LEVEL DEBUG_ALL

void logging_log(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);
void log_ISR(const char *str, uint32_t uptime, uint32_t uptime_ms, int level);

#define LOG_INTERRUPT(...) INTERFACE_printf(__VA_ARGS__)

// on FAILURE log does not work. Use LOG_INTERRUPT instead as native printf
#define LOG(level, ...)                                                   \
    if (level >= LOGGING_LEVEL)                                           \
    {                                                                     \
        snprintf(LOGGING_BUF, INTERFACE_BUFFER_SIZE, __VA_ARGS__);        \
        logging_log(LOGGING_BUF, reg_map_dyn.uptime_s, UPTIME_MS, level); \
    }

#define LOG_ISR(level, ...)                                               \
    if (level >= LOGGING_LEVEL)                                           \
    {                                                                     \
        snprintf(LOGGING_ISR_BUF, INTERFACE_BUFFER_SIZE, __VA_ARGS__);    \
        log_ISR(LOGGING_ISR_BUF, reg_map_dyn.uptime_s, UPTIME_MS, level); \
    }

void logging_init(osMutexId_t *logging_mutex, osMutexId_t *usb_cdc_mutex_arg);
void logging_send_to_interface();

void print_swo(const char *data, const uint32_t size);

extern osMutexId_t *logs_mutex;
extern osMutexId_t *usb_cdc_mutex;

#endif // LOGGING_H