import subprocess
import random
import time
import os
import sys

TYPES = [
    "int8_t", "uint8_t",
    "int16_t", "uint16_t",
    "int32_t", "uint32_t",
    "int64_t", "uint64_t",
    "float", "double"
]

RUNNER = "benchmarks/build/stress_test"
OUTPUT_DIR = "benchmarks/stress_failures"

if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

def run_stress_test():
    iteration = 0
    while True:
        iteration += 1
        type_name = random.choice(TYPES)
        size = random.randint(1, 1000000)

        filename = os.path.join(OUTPUT_DIR, f"input_{iteration}.bin")

        print(f"[{iteration}] Testing {type_name} with size {size}...", end="\r")

        cmd = [RUNNER, type_name, str(size), filename]

        try:
            result = subprocess.run(cmd, capture_output=True, text=True)

            if result.returncode != 0:
                print(f"\nFAILED: {type_name} size {size}")
                print(f"Stderr: {result.stderr}")
                print(f"Input saved to: {filename}")
                # Rename to indicate failure
                fail_filename = os.path.join(OUTPUT_DIR, f"FAILED_{type_name}_{size}_{iteration}.bin")
                os.rename(filename, fail_filename)
            else:
                # Success, remove file
                if os.path.exists(filename):
                    os.remove(filename)

        except Exception as e:
            print(f"\nException running test: {e}")
            break

if __name__ == "__main__":
    # Compile first
    print("Compiling stress test runner...")
    compile_cmd = [
        "g++", "-std=c++17", "-O3", "-pthread",
        "-I", "include", "-I", "benchmarks/include",
        "benchmarks/stress_test.cpp", "-o", "benchmarks/build/stress_test"
    ]
    try:
        subprocess.run(compile_cmd, check=True)
    except subprocess.CalledProcessError:
        print("Compilation failed!")
        sys.exit(1)

    print("Starting stress test (Ctrl+C to stop)...")
    try:
        run_stress_test()
    except KeyboardInterrupt:
        print("\nStress test stopped by user.")
