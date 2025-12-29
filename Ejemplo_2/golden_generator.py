import random
import csv
import numpy as np

# -----------------------------
# Configuration
# -----------------------------
VECTOR_LEN = 10            # Vector length
INPUT_BITSIZE = 12     # Bits per element
OUTPUT_BITSIZE = 32
NUM_SAMPLES = 1_000


OUTPUT_INPUT_FILE = "golden_inputs.csv"
OUTPUT_REF_FILE   = "golden_references.csv"

# -----------------------------
# Helper functions
# -----------------------------
def rand_value(bits, signed=False):
    """Generate a random integer within the allowed bit range."""
    # Range: 0 ... (2^bits - 1)
    if signed:
        return random.randint(-(1 << (bits-1)), (1 << (bits-1)) - 1)
    return random.randint(0, (1 << bits) - 1)

def intN(x, nbits=32):
    mask = (1 << nbits) - 1
    x &= mask
    sign_bit = 1 << (nbits - 1)
    if x & sign_bit:
        x -= (1 << nbits)
    return x

# -----------------------------
# Generate samples
# -----------------------------
all_inputs = []
all_refs = []
rng = np.random.default_rng()

for _ in range(NUM_SAMPLES):
    a = rng.integers(low=-(1 << (INPUT_BITSIZE-1)), high=(1 << (INPUT_BITSIZE-1)) -1, size=VECTOR_LEN)
    #a = [rand_value(INPUT_BITSIZE, signed=True) for _ in range(VECTOR_LEN)]
    #b = [rand_value(INPUT_BITSIZE, signed=True) for _ in range(VECTOR_LEN)]
    b = rng.integers(low=-(1 << (INPUT_BITSIZE-1)), high=(1 << (INPUT_BITSIZE-1)) -1, size=VECTOR_LEN)
    d = np.dot(a,b)

    # Save input (flattened for simple HDL parsing)
    all_inputs.append(list(a) + list(b))
    d = intN(d, OUTPUT_BITSIZE)
    all_refs.append(d)

# -----------------------------
# Write files
# -----------------------------
with open(OUTPUT_INPUT_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([f"x{i}" for i in range(VECTOR_LEN)] + [f"y{i}" for i in range(VECTOR_LEN)])
    for row in all_inputs:
        writer.writerow(row)

with open(OUTPUT_REF_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["Results"])
    for distance in all_refs:
        writer.writerow([distance])

print("Golden input and reference files generated.")

