CXX = g++
CXXFLAGS = -std=c++17 -O2 -march=native -DNDEBUG -Ibenchmarks/include -Iinclude
BUILD_DIR = benchmarks/build
SRC_DIR = benchmarks/src

# Windows native executables
RUNNER = $(BUILD_DIR)/benchmark_runner.exe
INTERACTIVE = $(BUILD_DIR)/interactive_runner.exe

all: runner interactive

runner:
ifeq ($(OS),Windows_NT)
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	mkdir -p $(BUILD_DIR)
endif
	$(CXX) $(CXXFLAGS) -o $(RUNNER) $(SRC_DIR)/benchmark_runner.cpp -pthread

interactive:
ifeq ($(OS),Windows_NT)
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	mkdir -p $(BUILD_DIR)
endif
	$(CXX) $(CXXFLAGS) -o $(INTERACTIVE) $(SRC_DIR)/interactive_runner.cpp -pthread

run: runner
	cd benchmarks && python benchmark_manager.py

clean:
ifeq ($(OS),Windows_NT)
	if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
else
	rm -rf $(BUILD_DIR)
endif

.PHONY: all runner interactive run clean
