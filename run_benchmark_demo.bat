@echo off
echo Running Dual-Pivot Quicksort Benchmark with Plotting
echo ====================================================

echo.
echo Building benchmark...
g++ -std=c++17 -Wall -Wextra -I./include -O3 -march=native -DNDEBUG -o results/benchmark_optimized.exe benchmarks/main_benchmark.cpp

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build successful!
echo.
echo Generating plots from existing benchmark data...
cd results
python ../scripts/plot_benchmark.py benchmark_results.csv

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✓ Plotting demonstration completed successfully!
    echo.
    echo Generated files:
    echo   - CSV: benchmark_results.csv
    echo   - Plots: plots/*.png
    echo.
    echo Plot types generated:
    echo   1. Individual performance plots for each data pattern
    echo   2. Summary comparison plot showing all patterns
    echo   3. Speedup analysis comparing algorithms vs std::sort
    echo.
    echo You can view the plots in the plots/ directory.
) else (
    echo Failed to generate plots. Make sure Python and matplotlib are installed.
    echo Run: pip install matplotlib pandas
)

echo.
pause