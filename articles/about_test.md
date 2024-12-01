# Тестирование библиотеки логирования для STM32 с использованием QEMU эмулятора и Docker

## Введение

Разработка встроенных систем на базе STM32 предъявляет высокие требования к надежности и эффективности логирования. Это критически важно для отладки и мониторинга приложений, особенно в условиях ограниченных ресурсов и реального времени. В данной статье рассмотрены основные вызовы, с которыми сталкиваются разработчики при интеграции и тестировании библиотек логирования, а также представлено решение на основе эмулятора QEMU и контейнеризации с помощью Docker.

### Оглавление

1. [Система сборки в Docker и эмулятор QEMU в Docker](#система-сборки-в-docker-и-эмулятор-qemu-в-docker)
2. [Тестовый проект](#тестовый-проект)
3. [Автоматическое тестирование](#автоматическое-тестирование)
4. [Отладка с использованием GDB и VS Code](#отладка-с-использованием-gdb-и-vs-code)

## Система сборки в Docker и эмулятор QEMU в Docker

Для обеспечения изолированности и воспроизводимости тестовой среды используется Docker. Все необходимые зависимости для сборки и запуска QEMU, а также для выполнения тестов, находятся внутри Docker-контейнеров. Это позволяет избежать проблем с несовместимостью библиотек и версий инструментов на локальных машинах разработчиков.

### Контейнеризация с Docker

Использование Docker обеспечивает стабильную и предсказуемую среду для разработки и тестирования. Основные компоненты тестовой среды включают:

1. **Dockerfile.tests**: Сборка ARM кросс-компиляторного инструментария. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/Dockerfile.tests)

2. **Dockerfile.qemu_stm32**: Сборка и настройка эмулятора QEMU с поддержкой STM32. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/Dockerfile.qemu_stm32)

3. **Dockerfile.run_tests**: Объединение компонентов для запуска тестов внутри контейнера. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/Dockerfile.run_tests)

### Docker Compose — Оркестрация контейнеров

Файл `compose.yaml` позволяет собрать Docker контейнеры одной командой, так как основной Dockerfile зависит от `qemu_stm32` и `tests`. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/compose.yaml)

## Тестовый проект

### Генерация проекта с помощью STM32CubeMX

Тестирование библиотеки осуществляется путем компиляции и запуска тестового проекта, сгенерированного с использованием [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html). Для платы **STM32-F103C8** выбрана опция генерации Makefile-проекта. В проекте настроены необходимые периферийные устройства, такие как таймеры, UART и часы, что позволяет запускать ядро FreeRTOS и проверять корректность работы с периферией.

### Интеграция библиотеки логирования и компиляция Makefile

Библиотека логирования копируется в тестовый проект, после чего осуществляется компиляция и запуск на эмуляторе STM32. Основные этапы включают настройку Makefile для включения библиотеки и обеспечение корректной сборки проекта.

Для копирования библиотеки и подготовки к сборке используется скрипт `test.sh`. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/test.sh)

```shell
# Удаление предыдущей версии библиотеки
rm -rf tests_stm32/logging_cmsis_rtos2
# Копирование библиотеки в тестовый проект
cp -r ../lib/ tests_stm32/logging_cmsis_rtos2

# Определение переменных пропущено

# Запуск Docker контейнера
docker compose up --build

# Получение кода завершения теста
exit_code=$(docker inspect -f '{{.State.ExitCode}}' "$test_container_name")
echo "exit_code = $exit_code"
exit "$exit_code"
```

## Автоматическое тестирование

Автоматизация тестирования позволяет быстро выявлять и исправлять ошибки, снижая время на ручное тестирование и повышая общую производительность команды разработчиков.

### Скрипты автоматизации

- **test_in_docker.sh**: Скрипт, выполняемый внутри контейнера `run_tests`. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/test_in_docker.sh)

```shell
    SERIAL_DEVICE=/dev/pts/0

    # Запуск QEMU в фоновом режиме
    qemu-system-arm -nographic -M stm32-f103c8 -kernel /app/firmware.bin -serial pty &
    QEMU_PID=$!

    # Ожидание регистрации сериал-устройства и запуска QEMU
    while [ ! -e "$SERIAL_DEVICE" ]; do
        if ! kill -0 "$QEMU_PID" 2>/dev/null; then
            echo "QEMU exited"
            exit 1
        fi
        sleep 0.1s
    done

    # Запуск проверки логов
    python3 verify_output.py "$SERIAL_DEVICE"
```

- **verify_output.py**: Python-скрипт для проверки соответствия логов ожидаемым шаблонам. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/verify_output.py)

```python
import re
import sys

def test_logging_basic_test(ser):
    # Чтение вывода из сериал-порта до нахождения новой строки

    # Проверка ожидаемых выводов от МК
    for i in range(100):
        output = ser.readline().decode('utf-8')
        # Соответствие шаблону "[INFO     ][0s.87]: basic_test_0"
        pattern = r"\[INFO\s*\]\[(\d+)s\.(\d+)\]: basic_test_(\d+)"

        match = re.match(pattern, output)

        if match:
            seconds = int(match.group(1))
            milliseconds = int(match.group(2))
            test_number = int(match.group(3))
            print(f"Test number: {test_number}, Time: {seconds}s.{milliseconds}")
            assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
            assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
        else:
            assert False, f"Output '{output}' does not match the expected pattern"

    print("Basic test of printing 100 logs in a row passed")
```

### GitHub Actions — Непрерывная интеграция

Для автоматизации процесса тестирования при каждом пуше или создании pull request в репозиторий настроен GitHub Actions. Файл конфигурации CI находится в `.github/workflows/test.yml`. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/.github/workflows/test.yml)

```yaml
name: CI

on: 
    push:
    pull_request

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
    - name: Checkout code
      uses: actions/checkout@v3

    - name: Run test
      run: cd test && ./test.sh

    - name: Archive test results
      if: always()
      uses: actions/upload-artifact@v4
      with:
        name: log
        path: ./test/logs.txt
```

Результаты тестирования доступны в разделе `Artifacts` на странице GitHub Actions.

## Отладка с использованием GDB и VS Code

<!-- TODO: Добавить информацию об отладке с использованием GDB remote, интеграции с VS Code, а также о профилировании FreeRTOS. -->

### Отладка с GDB Remote

*Комментарий:* Описание настройки отладки с использованием GDB remote через QEMU.

### Интеграция с VS Code

*Комментарий:* Инструкция по подключению VS Code к удаленной отладочной сессии через GDB.

### Профилирование FreeRTOS

*Комментарий:* Методы и инструменты для профилирования задач в FreeRTOS, а также анализ производительности системы.

## Непосредственно тестирование

### Основные функции библиотеки логирования

Библиотека логирования разработана с использованием FreeRTOS и CMSIS. Основные компоненты включают:

- **Логирование из обычных задач**: Макрос `LOG(level, ...)` позволяет записывать сообщения, соответствующие уровню логирования. Библиотека использует кольцевой буфер для хранения логов и мьютексы для синхронизации доступа.
  
- **Логирование из ISR**: Макрос `LOG_ISR(level, ...)` предназначен для использования в ISR. Он использует семафоры для безопасного добавления логов в буфер без блокировок.

### Примеры вывода логов в `tests.c`

Один из тестов `logging_basic_test` генерирует логи с уровнем `INFO` в цикле от 0 до 99. Каждый лог содержит имя теста и номер итерации.

```c
void logging_basic_test(char *name)
{
    for (int i = 0; i < 100; i++)
    {
        LOG(INFO, "%s_%d", name, i);
        osDelay(10);
    }
}
```

### Пример выводимого лога

При выполнении функции `logging_basic_test`, лог может выглядеть следующим образом:

```
[INFO     ][0s.87]: basic_test_0
[INFO     ][0s.88]: basic_test_1
...
[INFO     ][0s.98]: basic_test_99
```

### Парсинг логов в `verify_output.py`

Скрипт `verify_output.py` читает эти логи из сериал-порта и проверяет их соответствие ожидаемому формату с использованием регулярных выражений.

```python
def test_logging_basic_test(ser):
    # Read the output from the serial port until new line is found

    # Verify the expected prints from the MC
    for i in range(100):
        output = ser.readline().decode('utf-8')
        #  match this pattern like this  "[INFO     ][0s.87]: basic_test_0"
        # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
        pattern = r"\[INFO\s*\]\[(\d+)s\.(\d+)\]: basic_test_(\d+)"

        match = re.match(pattern, output)

        if match:
            seconds = int(match.group(1))
            milliseconds = int(match.group(2))
            test_number = int(match.group(3))
            print(f"Test number: {test_number}, Time: {seconds}s.{milliseconds}")
            assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
            assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
        else:
            assert False, f"Output '{output}' does not match the expected pattern"

    print("Basic test of printing 100 logs in a row passed")
```

### Список тестов

Функция `logging_test` запускает все тесты по очереди, проверяя корректность вывода логов в различных сценариях. Самым сложным является тест `logging_test_multiple_threads`, который запускает 10 задач, каждая из которых генерирует по 10 логов с уровнем `INFO`.

```c
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
```

#### Вывод логов из многопоточных тестов

```c
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
```

#### Парсинг многопоточных логов в `verify_output.py`

Скрипт `verify_output.py` также включает функцию для проверки логов из многопоточных тестов. [Смотреть файл](https://github.com/AleksandrVin/logging_cmsis_rtos2/blob/main/test/verify_output.py)

```python
def test_logging_multiple_threads(ser, threads=10):
    # create 10 threads which log at the same time
    names = ["logging_thread_" + str(i) for i in range(threads)]

    # Verify the expected prints from the MC

    # match this pattern like this "[INFO     ][0s.87]: logging_thread_X_Y"
    # where X is the thread number and Y is the log message number
    pattern = r"\[INFO\s*\]\[(\d+)s\.(\d+)\]: logging_thread_(\d+)_(\d+)"

    # Create a dictionary to store the logs for each thread
    logs = {name: [] for name in names}

    # Read the output from the serial port until new line is found
    for run in range(threads * 10):
        output = ser.readline().decode('utf-8')

        print(output)
        if not output:
            break

        match = re.match(pattern, output)
        if match:
            thread_number = int(match.group(3))
            log_message = int(match.group(4))
            logs[names[thread_number]].append(log_message)

    # Print the logs in a table format
    print("Thread Logs:")
    for name, log_messages in logs.items():
        print(f"{name}: {log_messages}")

    print("Test logging multiple threads passed")
```

## Преимущества выбранного подхода

### Повышение эффективности разработки

Автоматизированное тестирование значительно ускоряет процесс разработки, позволяя быстро выявлять и исправлять ошибки. Это снижает время на ручное тестирование и повышает общую производительность команды разработчиков.

### Гибкость и расширяемость

Использование Docker и QEMU предоставляет высокую изоляцию тестовой среды. Система может быть расширена для поддержки новых микроконтроллеров или периферийных устройств, а также может быть адаптирована под различные проекты.

## Ограничения и особенности подхода

Хотя данный метод позволяет значительно ускорить процесс тестирования и уменьшить зависимость от физического оборудования, он имеет несколько ограничений:

- **Ограниченная поддержка периферийных устройств**: QEMU не полностью поддерживает все периферийные устройства STM32, что может затруднить тестирование некоторых аспектов приложения.
- **Отсутствие полной эмуляции**: Некоторые специфические особенности железа могут не поддерживаться, что требует дополнительной проверки на реальном оборудовании.
- **Производительность**: Запуск эмулированных тестов может быть медленнее по сравнению с тестированием на реальном устройстве, особенно при выполнении сложных сценариев.

## Заключение

На рынке встроенных систем представлено огромное множество микроконтроллеров, что затрудняет создание качественных эмуляторов периферии. Разработка тестов для эмуляторов требует усилий, сравнимых с разработкой самой библиотеки. Этот проект представляет собой личный опыт и эксперимент по применению технологий CI/CD в мире встроенных систем.

## Дополнительные материалы

- [Исходный код проекта на GitHub](https://github.com/AleksandrVin/logging_cmsis_rtos2)
- [Статья на Habr](https://habr.com/ru/articles/814745/)
