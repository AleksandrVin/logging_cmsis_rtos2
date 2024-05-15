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
container="run_tests"
test_container_name="$service_name"_"$container"_1
test_container_name_alternative="$service_name"-"$container"-1

rm -rf $logs_file
docker-compose logs > $logs_file

set +e

exit_code=$(docker inspect -f '{{.State.ExitCode}}' $test_container_name)
if [ -z $exit_code ]; then
    exit_code=$(docker inspect -f '{{.State.ExitCode}}' $test_container_name_alternative)
fi
echo exit_code = $exit_code
exit $exit_code