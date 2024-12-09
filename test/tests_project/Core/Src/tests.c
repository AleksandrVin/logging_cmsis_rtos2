#include "logging.h"
#include "tests.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/**
 * Basis test
 *
 * One thread print logs from 1 to 100 without delay.
 */
void logging_basic_test(char *name)
{
    for (int i = 0; i < 100; i++)
    {
        LOG(INFO, "%s_%d", name, i);
        osDelay(10);
    }
}

void logging_log_sequence(char *name)
{
    for (int i = 0; i < LOGS_AT_ONCE; i++)
    {
        LOG(INFO, "%s_%d", name, i);
    }
}

void logging_test_pack(char *name)
{
    for (int i = 0; i < LOGS_AT_ONCE; i++)
    {
        logging_log_sequence(name);
        osDelay(10);
    }
}

void logging_test_different_levels(char *name)
{
    LOG(DEBUG_ALL, "%s_DEBUG_ALL", name)
    LOG(DEBUG_MIN, "%s_DEBUG_MIN", name);
    LOG(INFO, "%s_INFO", name);
    LOG(WARNING, "%s_WARNING", name);
    LOG(ERR, "%s_ERR", name);
}

void logging_test_fatal(char *name)
{
    for (int i = 0; i < LOGS_AT_ONCE; i++)
    {
        LOG_FATAL("%s_%d\n", name, i);
    }
}

void logging_thread(void *arg)
{
    const char *name = (const char *)arg;
    for (int i = 0; i < LOGS_AT_ONCE; i++)
    {
        LOG(INFO, "%s_%d", name, i);
        osThreadYield();
    }
    osThreadExit();
}

osThreadId_t threads[THREADS_AMOUNT];
char names[THREADS_AMOUNT][25];

void logging_test_multiple_threads()
{
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        snprintf(names[i], 25, "logging_thread_%d", i);
        osDelay(10);
    }
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        threads[i] = osThreadNew(logging_thread, names[i], NULL);
        if(threads[i] == NULL)
        {
            LOG_FATAL("ERROR: cannot create thread %d\n", i);
            Error_Handler();
        }
    }
    // wait for all threads to finish
    osDelay(1000);
    // check if all threads finished
    for (int i = 0; i < THREADS_AMOUNT; i++)
    {
        if(osThreadGetState(threads[i]) != osThreadTerminated)
        {
            LOG_FATAL("ERROR: thread %d did not finish\n", i);
            Error_Handler();
        }
    }
    LOG_FATAL("All threads finished\n");
}

void logging_interrupt()
{
    // only last one will be printed
    for (int i = 0; i < LOGS_AT_ONCE; i++)
    {
        LOG_ISR(INFO, "LOG_ISR_%d", i);
    }
}

void logging_test()
{
    logging_basic_test("basic_test");
    logging_test_pack("pack_of_ten_ten_times");
    logging_test_different_levels("different_levels");
    osDelay(200); // set log pass
    logging_test_fatal("fatal");
    osDelay(200); // log pass
    logging_interrupt();
    osDelay(200); // log pass
    logging_test_multiple_threads();
}