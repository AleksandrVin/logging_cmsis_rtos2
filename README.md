# logging_cmsis_rtos2
Logging library for cmsis_rtos2 operating system with delayed logging ability

This library helps to create redundant logging facility on stm32 microcontrollers with cmsis_rtos2 operating system.
Algorithm is based on simplified printk function from linux kernel. It uses ring buffer to store log messages

## How to use

1. Update makefile to logging.h file in your project
2. Update makefile to include logging.c file location in your project
3. Resolve logging.h file dependencies in your project
4. Provide physical interface functions for custom interfaces. See weak functions in logging.h file for examples
5. Select timer for logging. It will be initialized by logging_init() function
6. Select uart for logging. It will be initialized by logging_init() function
7. Select usb_cdc interface for logging. It will be initialized by logging_init() function
8. Call logging_init() function before any logging function call. Provide timer pointer and at least one interface pointer

## This is low level library

Logging thread runs with minimum priority. It means that logging thread will not be able to log messages if there is no free CPU time.
It is recommended to use mutexes, semaphores, queues and other synchronization primitives to avoid deadlocks and free some time for logging.