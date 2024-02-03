import sys
import re

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

def test_logging_test_pack(ser):

    # Verify the expected prints from the MC
    for i in range(10):
        for j in range(10):
            output = ser.readline().decode('utf-8')

            #  match this pattern like this  "[INFO     ][0s.87]: pack_of_ten_ten_times_0"
            # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
            pattern = r"\[INFO\s*\]\[(\d+)s\.(\d+)\]: pack_of_ten_ten_times_(\d+)"
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

    print("Test pack of ten ten times passed")

def test_logging_different_levels(ser):
    # Verify the expected prints from the MC
    output = ser.readline().decode('utf-8')

    # match this pattern like this  "[DEBUG_ALL][0s.87]: name_DEBUG_ALL"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"\[DEBUG_ALL\]\[(\d+)s\.(\d+)\]: different_levels_DEBUG_ALL"
    match = re.match(pattern, output)

    if match:
        seconds = int(match.group(1))
        milliseconds = int(match.group(2))
        print(f"Time: {seconds}s.{milliseconds} level: DEBUG_ALL")
        assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
        assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
    else:
        assert False, f"Output '{output}' does not match the expected pattern"

    output = ser.readline().decode('utf-8')

    # match this pattern like this  "[DEBUG_MIN][0s.87]: name_DEBUG_MIN"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"\[DEBUG_MIN\]\[(\d+)s\.(\d+)\]: different_levels_DEBUG_MIN"
    match = re.match(pattern, output)

    if match:
        seconds = int(match.group(1))
        milliseconds = int(match.group(2))
        print(f"Time: {seconds}s.{milliseconds} level: DEBUG_MIN")
        assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
        assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
    else:
        assert False, f"Output '{output}' does not match the expected pattern"

    output = ser.readline().decode('utf-8')

    # match this pattern like this  "[INFO     ][0s.87]: name_INFO"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"\[INFO\s*\]\[(\d+)s\.(\d+)\]: different_levels_INFO"
    match = re.match(pattern, output)

    if match:
        seconds = int(match.group(1))
        milliseconds = int(match.group(2))
        print(f"Time: {seconds}s.{milliseconds} level: INFO")
        assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
        assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
    else:
        assert False, f"Output '{output}' does not match the expected pattern"

    output = ser.readline().decode('utf-8')

    # match this pattern like this  "[WARNING  ][0s.87]: name_WARNING"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"\[WARNING\s*\]\[(\d+)s\.(\d+)\]: different_levels_WARNING"
    match = re.match(pattern, output)

    if match:
        seconds = int(match.group(1))
        milliseconds = int(match.group(2))
        print(f"Time: {seconds}s.{milliseconds} level: WARNING")
        assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
        assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
    else:
        assert False, f"Output '{output}' does not match the expected pattern"

    output = ser.readline().decode('utf-8')

    # match this pattern like this  "[ERR      ][0s.87]: name_ERR"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"\[ERROR\s*\]\[(\d+)s\.(\d+)\]: different_levels_ERR"
    match = re.match(pattern, output)

    if match:
        seconds = int(match.group(1))
        milliseconds = int(match.group(2))
        print(f"Time: {seconds}s.{milliseconds} level: ERROR")
        assert seconds >= 0, f"Seconds '{seconds}' should be greater than 0"
        assert milliseconds >= 0, f"Milliseconds '{milliseconds}' should be greater than 0"
    else:
        assert False, f"Output '{output}' does not match the expected pattern"

    print("Test different levels passed")

def test_logging_interrupt(ser):
    # Verify the expected prints from the MC

    # match this pattern like this "interrupt_0"
    # where 0s.78 is the time in seconds and milliseconds that should be stored for analysis
    pattern = r"interrupt_(\d+)"

    for i in range(10):
        output = ser.readline().decode('utf-8')
        match = re.match(pattern, output)

        if match:
            number = int(match.group(1))
            assert number == i, f"Number '{number}' should be equal to '{i}'"
        else:
            assert False, f"Output '{output}' does not match the expected pattern"

    print("Test interrupt passed")

def test_logging_test():
    # get serial device path from first argument
    ser = None
    if len(sys.argv) > 1:
        serial_device = sys.argv[1]
        ser = open(serial_device, 'rb+', buffering=0)
    else:
        exit(1)

    test_logging_basic_test(ser)
    test_logging_test_pack(ser)
    test_logging_different_levels(ser)
    test_logging_interrupt(ser)

    ser.close()

test_logging_test()
            
