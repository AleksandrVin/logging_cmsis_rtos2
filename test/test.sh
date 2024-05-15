#!/bin/bash

# this script copies logging library to test project to its proper location
# this is needed for test purpose, but also shows how to add it to your project

set -e

rm -rf tests_stm32/logging_cmsis_rtos2
cp -r ../lib/ tests_stm32/logging_cmsis_rtos2

# start docker container
docker-compose up --build

logs_file="logs.txt"
compose_file="compose.yaml"
service_name="test"
container="run_tests-1"
test_container_name="$service_name_$container"

rm -rf $logs_file
docker-compose logs > $logs_file
exit_code=$(docker inspect -f '{{.State.ExitCode}}' $test_container_name)
echo exit_code = $exit_code
exit $exit_code