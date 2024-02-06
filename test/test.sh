#!/bin/bash

# this script copies logging library to test project to its proper location
# this is needed for test purpose, but also shows how to add it to your project

set -e

rm -rf tests_stm32/logging_cmsis_rtos2
cp -r ../lib/ tests_stm32/logging_cmsis_rtos2

# start docker container
docker-compose up --build