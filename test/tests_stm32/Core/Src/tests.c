#include "logging.h"

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
    for (int i = 0; i < 10; i++)
    {
        LOG(INFO, "%s_%d", name, i);
    }
}

void logging_test_pack(char *name)
{
    for (int i = 0; i < 10; i++)
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

void logging_test_interrupt(char *name)
{
    for (int i = 0; i < 10; i++)
    {
        LOG_FATAL("%s_%d\n", name, i);
    }
}

void logging_test()
{
    logging_basic_test("basic_test");
    logging_test_pack("pack_of_ten_ten_times");
    logging_test_different_levels("different_levels");
    osDelay(50); // set interrupt log pass
    logging_test_interrupt("interrupt");
}