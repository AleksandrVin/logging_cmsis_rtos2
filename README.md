# logging_cmsis_rtos2

Logging library for cmsis_rtos2 operating system with delayed logging ability

## About

[About](articles/about_lib.md)

This library helps to create redundant logging facility on stm32 microcontrollers with cmsis_rtos2 operating system.
Algorithm is based on simplified printk function from linux kernel. It uses ring buffer to store log messages

This library meant to be used with uart or virtual com port interface, but can be easily adapted to other interfaces
Special attention was paid to ensure that when usb_cdc interface is used, logging thread will block until transaction is completed
if it is configured to do so.

## How to use

1. Update makefile to logging.h file in your project
2. Update makefile to include logging.c file location in your project
3. Resolve logging.h file dependencies in your project
4. If necessary provide your own print function for your custom interface. This function must block until transaction is completed
5. Configure defines in logging.h for your needs
6. Call logging_init() function before any logging function call

## This is low level library

Logging thread runs with minimum priority. It means that logging thread will not be able to log messages if there is no free CPU time.
It is recommended to use mutexes, semaphores, queues and other synchronization primitives to avoid deadlocks and free some time for logging.

## Tests

[About tests](articles/about_test.md)

Tests are executed using [qemu-stm32](https://github.com/beckus/qemu_stm32) implementation. 
[Other qemu examples](https://github.com/beckus/stm32_p103_demos/tree/master)

This approach is quite tricky and might produce wrong results, though it is suitable for basic evaluation. 

### Running test on qemu

```bash
cd test
./test.sh
```

This script will compile test project with logging library. Start it in qemu emulator, then output from virtual uart 
will be verified with python script
