import logging
import time
import csv
import numpy as np
from serial import Serial, PARITY_NONE, STOPBITS_ONE
import sklearn.metrics as metrics

# Logging setup
logging.basicConfig(
    level=logging.INFO,  # Change to logging.INFO to reduce verbosity
    format='[%(asctime)s] %(levelname)s: %(message)s',
    datefmt='%H:%M:%S'
)
logger = logging.getLogger(__name__)

PORT = "/dev/ttyUSB1"
BAUDRATE = 115200
STOP_BITS = STOPBITS_ONE
PARITY = PARITY_NONE

TEST_LEN = 100
I_FILE = "golden_inputs.csv"        # one row = full input vector per test
R_FILE = "golden_references.csv"   # one row = reference output per test
O_FILE = "hardware_Results.txt"
CSV_FILE = "results_log.csv"

HEADERS = ["Hardware", "GoldenReference", "AbsoluteError", "PercentageError", "IterationTime"]
ABS_TOL = 1
PER_TOL = 10
VECTOR_SIZE = 10   # expects 2*VECTOR_SIZE inputs per row

input_data = []
output_data = []
true_data = []
time_data = []

tst_failed = 0

logger.info(f"Opening serial port {PORT} at {BAUDRATE} baud.")
ser = Serial(port=PORT, baudrate=BAUDRATE, parity=PARITY, stopbits=STOP_BITS)
logger.info("Listening for board...")

attempts = 0
while True:
    line = ser.readline().decode('ascii', errors='ignore').strip()
    if line == "Running! ...":
        break
    if attempts > 10:
        logger.error("No board detected!")
        ser.close()
        exit(1)
    attempts += 1
    logger.debug(f"Attempt {attempts}: Waiting for board response...")
    time.sleep(1)

logger.info("Board detected!")

with open(CSV_FILE, 'w', newline='', encoding='utf8') as csvf, \
     open(I_FILE, 'r', encoding='utf8') as ifile, \
     open(R_FILE, 'r', encoding='utf8') as rfile:

    writer = csv.writer(csvf)
    writer.writerow(HEADERS)

    ireader = csv.reader(ifile)
    rreader = csv.reader(rfile)

    # Skip headers if present
    next(ireader, None)
    next(rreader, None)

    for tst, (irow, rrow) in enumerate(zip(ireader, rreader)):
        if tst >= TEST_LEN:
            break

        logger.debug(f"Test ID: {tst}")

        # Parse input vector from one CSV row
        data_in = [float(x) for x in irow]
        if len(data_in) != 2 * VECTOR_SIZE:
            raise ValueError(f"Test {tst}: Expected {2*VECTOR_SIZE} inputs, got {len(data_in)}")

        input_data.append(data_in)

        # Parse reference output (assume first column)
        data_ref = float(rrow[0])
        true_data.append(data_ref)

        # Send inputs to device
        for item in data_in:
            ser.write(f"{item}\n".encode("ascii"))
            logger.debug(f"Sent to device: {item}")
            time.sleep(0.01)

        logger.debug("Waiting for device response...")
        oline = ser.readline().decode("ascii", errors='ignore').strip()
        logger.debug(f"Received line: {oline}")
        tline = ser.readline().decode("ascii", errors='ignore').strip()
        logger.debug(f"Received line: {tline}")

        # Expect something like "Result: <value>"
        data_out = float(oline.split(":")[1])
        output_data.append(data_out)

        time_sw = float(tline.strip().split(":")[1].split(" ")[1]) * 2 / 0.65
        time_data.append(time_sw)

        abs_err = abs(data_out - data_ref)
        per_err = 100 * abs_err / (abs(data_ref) + 1e-16)

        logger.info(
            f"TEST: {tst}, Expected: {data_ref:.3f}, Got: {data_out:.3f}, "
            f"AbsErr: {abs_err:.3f}, PerErr: {per_err:.3f}, TIME: {time_sw} ns"
        )

        with open(O_FILE, "a", encoding="utf8") as ofile:
            ofile.write(
                f"TEST: {tst},\t Expected: {data_ref:6f} \tGot: {data_out:6f} "
                f"\t AbsErr: {abs_err:6f} \t PerErr: {per_err:6f}, TIME: {time_sw} ns\n"
            )

        if abs_err > ABS_TOL or per_err > PER_TOL:
            tst_failed += 1
            logger.warning(f"TEST {tst} Failed!")

        writer.writerow([data_out, data_ref, abs_err, per_err, time_sw])

# Metrics
mse = metrics.mean_squared_error(np.array(true_data), np.array(output_data))
mae = metrics.mean_absolute_error(np.array(true_data), np.array(output_data))

# Time Metrics
avg_time = np.mean(time_data)
max_time = np.max(time_data)
min_time = np.min(time_data)
std_time = np.std(time_data)

logger.info("=" * 60)
logger.info(f"{'Test Summary':^60}")
logger.info("=" * 60)
logger.info(f"{'Total Tests':<30}: {len(true_data)}")
logger.info(f"{'Failed Tests':<30}: {tst_failed}")
logger.info(f"{'MSE':<30}: {mse:.6f}")
logger.info(f"{'MAE':<30}: {mae:.6f}")
logger.info(f"{'Average Time (ns)':<30}: {avg_time:.6f}")
logger.info(f"{'Max Time (ns)':<30}: {max_time:.6f}")
logger.info(f"{'Min Time (ns)':<30}: {min_time:.6f}")
logger.info(f"{'Time std':<30}: {std_time:.6f}")
logger.info("=" * 60)

if tst_failed:
    logger.error(f"{'RESULT: FAIL':^60}")
else:
    logger.info(f"{'RESULT: PASS':^60}")
logger.info("=" * 60)

ser.close()
