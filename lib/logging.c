#include "logging.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "string.h"

char LOGGING_BUF[LOG_BUFFER_SIZE] = "\0";
char LOGGING_ISR_BUF[LOG_BUFFER_SIZE] = "\0";
char INTERFACE_BUFFER[INTERFACE_BUFFER_SIZE] = "\0";

char log_circular_buf[LOG_QUEUE_ROWS][LOG_BUFFER_SIZE] = {0};
uint32_t log_time_buf[LOG_QUEUE_ROWS] = {0};
uint32_t log_time_ms_buf[LOG_QUEUE_ROWS] = {0};
int log_level_buf[LOG_QUEUE_ROWS] = {0};
int logs_tail = 0;
int logs_head = 0;

int log_isr_set = 0;
uint32_t log_isr_time = 0;
uint32_t log_isr_time_ms = 0;
int log_isr_level = 0;

char log_names[ERR + 1][LOG_TYPE_SIZE] = {"DEBUG_ALL", "DEBUG_MIN", "INFO     ", "WARNING  ", "ERROR    "};

osMutexId_t interface_mutex;
osMutexId_t logs_mutex;

/* Definitions for logs semaphore */
osSemaphoreId_t logs_semaphore_store;
osSemaphoreId_t logs_semaphore_print;

typedef StaticTask_t osStaticThreadDef_t;

osThreadId_t loggingTaskHandle;
osThreadAttr_t loggingTask_attributes;

__NO_RETURN void StartLoggingTask(void *argument)
{
    while (1)
    {
        logging_send_to_interface();
        osThreadYield();
    }
}

void logging_init()
{
    logs_tail = 0;
    logs_head = 0;

    interface_mutex = osMutexNew(NULL);
    if (interface_mutex == NULL)
    {
        LOG_FATAL("ERROR: cannot create interface_mutex\n");
        Error_Handler();
    }
    logs_mutex = osMutexNew(NULL);
    if (logs_mutex == NULL)
    {
        LOG_FATAL("ERROR: cannot create logs_mutex\n");
        Error_Handler();
    }
    logs_semaphore_store = osSemaphoreNew(LOG_SEMAPHORE_COUNT_MAX, LOG_SEMAPHORE_COUNT_MAX, NULL);
    if (logs_semaphore_store == NULL)
    {
        LOG_FATAL("ERROR: cannot create logs_semaphore_store\n");
        Error_Handler();
    }
    logs_semaphore_print = osSemaphoreNew(LOG_SEMAPHORE_COUNT_MAX, 0, NULL);
    if (logs_semaphore_print == NULL)
    {
        LOG_FATAL("ERROR: cannot create logs_semaphore_print\n");
        Error_Handler();
    }

    // create thread with min priority to handle logging
    memset(&loggingTask_attributes, 0, sizeof(loggingTask_attributes));
    loggingTask_attributes.priority = osPriorityLow;
    loggingTaskHandle = osThreadNew(StartLoggingTask, NULL, &loggingTask_attributes);
}

/// @brief this function can log only one message at a time
/// and should be called from ISR(from interrupt) because normal logging with mutex is not allowed from ISR
/// @param str
/// @param uptime
void log_ISR(const char *str, uint32_t uptime, uint32_t uptime_ms, int level)
{
    if (osSemaphoreAcquire(logs_semaphore_store, 0) == osOK)
    {
        log_isr_set = 1;
        log_isr_time = uptime;
        log_isr_time_ms = uptime_ms;
        log_isr_level = level;
        osSemaphoreRelease(logs_semaphore_print);
    }
}

void logging_log(const char *str, uint32_t uptime, uint32_t uptime_ms, int level)
{
    // check if logging is initialized
    if (logging_is_initialized() == 0)
    {
        return;
    }
    osStatus_t status = osSemaphoreAcquire(logs_semaphore_store, LOG_MUTEX_TIMEOUT);
    if (status != osOK)
    {
        LOG_FATAL("ERROR log: cannot acquire logs_semaphore_store: %d\n", status);
        return;
    }
    status = osMutexAcquire(logs_mutex, LOG_MUTEX_TIMEOUT);
    if (status != osOK)
    {
        if (status == osErrorTimeout)
        {
            LOG_FATAL("logging_log timeout\n");
        }
        else
        {
            LOG_FATAL("ERROR log: cannot acquire logs_mutex: %d\n", status);
        }
        osSemaphoreRelease(logs_semaphore_store);
        return;
    }
    strncpy(log_circular_buf[logs_tail], str, LOG_BUFFER_SIZE);
    log_time_buf[logs_tail] = uptime;
    log_time_ms_buf[logs_tail] = uptime_ms;
    log_level_buf[logs_tail] = level;

    if (logs_tail < LOG_QUEUE_ROWS - 1)
    {
        logs_tail++;
    }
    else
    {
        logs_tail = 0;
    }
    osMutexRelease(logs_mutex);
    osSemaphoreRelease(logs_semaphore_print);
}

void logging_send_to_interface()
{
    // try to get logs semaphore
    if (osSemaphoreAcquire(logs_semaphore_print, osWaitForever) != osOK)
    {
        LOG_FATAL("ERROR send: cannot acquire logs_semaphore_print\n");
        return;
    }
    osStatus_t status = osError;
    status = osMutexAcquire(interface_mutex, osWaitForever);
    if (status != osOK)
    {
        LOG_FATAL("ERROR send: can't acquire usb_cdc_mutex:%d\n", status);
        osSemaphoreRelease(logs_semaphore_print);
        Error_Handler();
    }
    status = osMutexAcquire(logs_mutex, osWaitForever);
    if (status != osOK)
    {
        LOG_FATAL("ERROR send: cannot acquire logs_mutex: %d\n", status);
        osMutexRelease(interface_mutex);
        osSemaphoreRelease(logs_semaphore_print);
        Error_Handler();
    }

    if (log_isr_set)
    {
        INTERFACE_printf(FATAL_FLAG_CLEAR, "[%s][%lds.%ld]ISR: %s\n", log_names[log_isr_level], log_isr_time, log_isr_time_ms, LOGGING_ISR_BUF);
        log_isr_set = 0;
    }
    else
    {
        INTERFACE_printf(FATAL_FLAG_CLEAR, "[%s][%lds.%ld]: %s\n", log_names[log_level_buf[logs_head]], log_time_buf[logs_head], log_time_ms_buf[logs_head], log_circular_buf[logs_head]);
        if (logs_head < LOG_QUEUE_ROWS - 1)
        {
            logs_head++;
        }
        else
        {
            logs_head = 0;
        }
    }
    osMutexRelease(logs_mutex);
    osMutexRelease(interface_mutex);
    osSemaphoreRelease(logs_semaphore_store);
}

void print_swo(const char *data, const uint32_t size)
{
    uint32_t i;
    for (i = 0; i < size; i++)
    {
        ITM_SendChar(data[i]);
    }
}

int logging_is_initialized()
{
    return (interface_mutex != NULL) && (logs_mutex != NULL) && (logs_semaphore_store != NULL) && (logs_semaphore_print != NULL);
}