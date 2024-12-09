#!/bin/bash

# this script copies logging library to test project to its proper location
# this is needed for test purpose, but also shows how to add it to your project

set -e

rm -rf test_project/logging_cmsis_rtos2
cp -r ../lib/ test_project/logging_cmsis_rtos2

logs_dir="logs"
logs_file="$logs_dir/logs.txt"
compose_file="compose.yaml"
service_name="test"
container="run_tests"
test_container_name="$service_name"_"$container"_1
test_container_name_alternative="$service_name"-"$container"-1

rm -rf $logs_dir
rm -rf test_project/build/

# start docker container
docker compose --profile test up --build

# folder will be created by docker compose
docker compose logs $container > $logs_file

set +e

exit_code=$(docker inspect -f '{{.State.ExitCode}}' $test_container_name 2>/dev/null) 
if [ -z "$exit_code" ]; then
    exit_code=$(docker inspect -f '{{.State.ExitCode}}' $test_container_name_alternative 2>/dev/null)
fi
echo exit_code = $exit_code
exit $exit_code
