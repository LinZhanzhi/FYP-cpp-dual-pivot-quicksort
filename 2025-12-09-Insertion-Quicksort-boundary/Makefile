CXX = g++
CXXFLAGS = -O3 -march=native -std=c++17 -I src
TARGET = build/benchmark
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf build
