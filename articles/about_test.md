# Тестирование библиотеки логирования для STM32 с использованием QEMU эмулятора и Docker

## Рекомендации

Для разработчиков встроенных систем, пользователей STM32, специалистов по тестированию и всех, кто заинтересован в автоматизации процессов разработки, рекомендуется ознакомиться с исходным кодом в репозитории на [GitHub](https://github.com/AleksandrVin/logging_cmsis_rtos2) и с тестируемой библиотекой в статье на [Habr](https://habr.com/ru/articles/814745/). Это поможет лучше понять структуру проекта и организацию тестовых папок.

## Введение

Когда я начинал работу над разработкой встроенных систем на базе STM32, одной из ключевых задач было обеспечение надежного и эффективного логирования. Логирование играет критически важную роль в отладке и мониторинге приложений, особенно в условиях ограниченных ресурсов и реального времени. Однако, как и многие разработчики, я столкнулся с рядом проблем при интеграции и тестировании библиотеки логирования:

- **Ограниченные ресурсы аппаратных средств:** Работа с микроконтроллерами STM32 требует оптимального использования ресурсов, и любая избыточность в логировании может привести к снижению производительности.
- **Сложность в тестировании на реальном оборудовании:** Проверка работы логирования на физическом устройстве может быть затруднительной, особенно если оборудование недоступно или требует длительной настройки.
- **Необходимость автоматизации процесса тестирования:** Для обеспечения качества и надежности системы требуется автоматизированный подход к тестированию, который минимизирует ручные вмешательства и ускоряет процесс разработки.

Именно эти вызовы побудили меня разработать и протестировать новую библиотеку логирования для STM32, используя эмулятор QEMU и контейнеризацию через Docker. Моя цель состояла в создании надежной системы логирования, которая не только соответствует требованиям по производительности, но и легко интегрируется в существующие процессы разработки и тестирования.

В этой статье я поделюсь своим опытом в тестировании библиотеки логирования, расскажу о выбранных инструментах и подходах, а также о том, какие результаты удалось достичь. Независимо от того, являетесь ли вы разработчиком встроенных систем, пользователем STM32 или специалистом по тестированию, надеюсь, этот материал будет для вас полезен и вдохновит на внедрение эффективных методов автоматизации в ваших проектах.

## Генерация тестового проекта с помощью STM32CubeMX

Тестирование библиотеки происходит путем компиляции и запуска тестового проекта. Библиотека копируется в тестовый проект, после чего компилируется и запускается на эмуляторе STM32.

Проект для тестирования STM32 был сгенерирован с использованием [STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html). При создании проекта была выбрана опция генерации Makefile-проекта для платы **STM32-F103C8**, которая оснащена распространённым ARM Cortex-M3 чипом, поддерживаемым по умолчанию в QEMU. В проекте были настроены необходимые периферийные устройства, такие как таймеры, UART, часы и другие, что позволяет не только запустить ядро FreeRTOS, но и проверить корректность работы с периферией. В нашем случае нас интересует только UART, который используется для вывода логов. Список поддерживаемой периферии для stm32f103c8 можно проверить в форке QEMU-репозитория [beckus/qemu_stm32](https://github.com/beckus/qemu_stm32.git).

## QEMU — Эмуляция аппаратных компонентов

[QEMU](https://www.qemu.org/) — это мощный эмулятор, который позволяет запускать виртуальные машины с различными архитектурами процессоров. В контексте разработки встроенных систем QEMU предоставляет возможность эмулировать микроконтроллеры, такие как STM32. Это особенно полезно для автоматизированного тестирования и отладки, так как позволяет быстро запускать и повторять тесты в изолированной среде.

Для поддержки эмуляции STM32 использована модифицированная версия QEMU, доступная в репозитории [beckus/qemu_stm32](https://github.com/beckus/qemu_stm32.git). Этот форк QEMU включает специфические настройки и патчи, необходимые для корректной работы с микроконтроллерами STM32. Основные особенности модификации включают:

## Контейнеризация с Docker

Использование Docker для контейнеризации тестовой среды обеспечивает изолированность и воспроизводимость тестов. Все зависимости, необходимые для сборки и запуска QEMU, а также для выполнения тестов, находятся внутри Docker-контейнеров. Это позволяет избежать проблем с несовместимостью библиотек и версий инструментов на локальных машинах разработчиков.

### Основные компоненты тестовой среды

1. **Dockerfile.tests**: Этот Dockerfile отвечает за сборку ARM кросс-компиляторного инструментария, необходимого для компиляции тестовой прошивки. Он загружает и устанавливает `arm-none-eabi` toolchain, обеспечивая среду для сборки проектов на базе STM32.
    
2. **Dockerfile.qemu_stm32**: Этот Dockerfile используется для сборки и настройки эмулятора QEMU с поддержкой STM32. Он включает все необходимые зависимости и собирает модифицированную версию QEMU из репозитория [beckus/qemu_stm32](https://github.com/beckus/qemu_stm32.git).
    
3. **Dockerfile.run_tests**: Этот Dockerfile объединяет компоненты для запуска тестов внутри контейнера.

### Скрипты автоматизации

- **test.sh**: Основной скрипт для управления процессом тестирования. Он копирует библиотеку логирования в проект, запускает Docker Compose для выполнения тестов, а затем собирает и анализирует логи после окончания тестов.

    ```shell
        rm -rf tests_stm32/logging_cmsis_rtos2
        cp -r ../lib/ tests_stm32/logging_cmsis_rtos2
    
        # variable definitions skipped
    
        # Запуск Docker контейнера
        docker compose up --build
    
        exit_code=$(docker inspect -f '{{.State.ExitCode}}' "$test_container_name")
        if [ -z "$exit_code" ]; then
            exit_code=$(docker inspect -f '{{.State.ExitCode}}' "$test_container_name_alternative")
        fi
        echo "exit_code = $exit_code"
        exit "$exit_code"
    ```

- **test_in_docker.sh**: Скрипт, выполняемый внутри контейнера `run_tests`. Он запускает QEMU в фоновом режиме, ожидает инициализации сериал-порта и запускает Python-скрипт для проверки выводимых логов.

    ```shell
        SERIAL_DEVICE=/dev/pts/0
    
        # Запуск QEMU в фоновом режиме
        qemu-system-arm -nographic -M stm32-f103c8 -kernel /app/firmware.bin -serial pty  &
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

- **verify_output.py**: Python-скрипт, который читает логи из сериал-порта и проверяет их соответствие ожидаемым шаблонам.

### Docker Compose — Оркестрация контейнеров

Файл `compose.yaml` позволяет собрать докер контейнеры одной командой, т.к. основной Dockerfile зависит от qemu_stm32 и tests.

## Автоматизация тестирования с GitHub Actions

### Интеграция с GitHub Actions

Для автоматизации процесса тестирования при каждом пуше или создании pull request в репозиторий, настроен GitHub Actions. Файл конфигурации CI находится в `.github/workflows/test.yml` и выглядит следующим образом. Благодаря test.sh и Docker можно использовать одну конфигурацию для локального тестирования и для CI/CD:

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

## Непосредственно тестирование

### Основные функции библиотеки логирования

Библиотека логирования разработана с использованием FreeRTOS и CMSIS. Основными компонентами являются:

- **Логирование из обычных задач**: Макрос `LOG(level, ...)` позволяет записывать сообщения, соответствующие уровню логирования. Библиотека использует кольцевой буфер для хранения логов и мьютексы для синхронизации доступа.
  
- **Логирование из ISR**: Макрос `LOG_ISR(level, ...)` предназначен для использования в ISR. Он использует семафоры для безопасного добавления логов в буфер без блокировок.

В этом разделе приведены примеры того, как логи выводятся в файле `tests.c` и как они обрабатываются и проверяются в `verify_output.py`.

После запуска планировщика FreeRTOS управление передается в функцию которая запускает тесты поочередно.

### Вывод логов в `tests.c`

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

```shell
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

#### Список тестов

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

##### Вывод логов из многопоточных тестов

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
````

#### Парсинг многопоточных логов в `verify_output.py`

```python
def test_logging_multiple_threads(ser, threads=10):
    # create 10 thread which logs at the same time
    # use dynamic allocation
    names = ["logging_thread_" + str(i) for i in range(threads)]

    # Verify the expected prints from the MC

    # match this pattern like this "logging_thread_X: Log message"
    # where X is the thread number and Log message is the actual log message
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

### Управление приоритетами и патчи

В процессе тестирования обнаружена проблема с приоритетами системных вызовов (`syscall priority`) в эмуляторе QEMU. Для обхода этой проблемы был применён патч к файлу `port.c`. Это временное решение, и далее планируется внесение исправлений в форк QEMU.

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

Т.к. на рынке встраиваемых систем очень большое многообразие микроконтроллеров, то создание качественных эмуляторов периферии затруднительно. Также, разработка тестов для эмуляторов требует соизмеримых с разработкой самой библиотеки усилий. Этот проект — мой личный опыт и эксперимент, в котором я пытался применить технологии CI/CD в мире встроенных систем.

## Дополнительные материалы

- [Исходный код проекта на GitHub](https://github.com/AleksandrVin/logging_cmsis_rtos2)
- [Статья на Habr](https://habr.com/ru/articles/814745/)
