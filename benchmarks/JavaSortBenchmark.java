import java.util.Arrays;
import java.util.Random;

public class JavaSortBenchmark {
    public static void main(String[] args) {
        final int SIZE = 10_000_000;
        final int ITERATIONS = 30;

        // Generate random data
        Random random = new Random(42); // Fixed seed for reproducibility
        int[] original = new int[SIZE];
        for (int i = 0; i < SIZE; i++) {
            original[i] = random.nextInt();
        }

        // Warmup JIT (5 runs)
        System.out.println("Warming up JIT...");
        for (int i = 0; i < 5; i++) {
            int[] warmup = original.clone();
            Arrays.sort(warmup);
        }

        // Benchmark
        System.out.println("Running " + ITERATIONS + " iterations on " + SIZE + " integers...");
        double[] times = new double[ITERATIONS];

        for (int i = 0; i < ITERATIONS; i++) {
            int[] data = original.clone();

            long start = System.nanoTime();
            Arrays.sort(data);
            long end = System.nanoTime();

            times[i] = (end - start) / 1_000_000.0; // Convert to ms

            // Verify sorted
            if (i == 0 && !isSorted(data)) {
                System.err.println("ERROR: Array not sorted!");
                System.exit(1);
            }

            System.out.printf("  Iteration %2d: %.3f ms%n", i + 1, times[i]);
        }

        // Calculate statistics
        double min = Arrays.stream(times).min().orElse(0);
        double max = Arrays.stream(times).max().orElse(0);
        double avg = Arrays.stream(times).average().orElse(0);
        double median = getMedian(times);

        System.out.println("\n=== Results ===");
        System.out.printf("Size:       %,d integers%n", SIZE);
        System.out.printf("Iterations: %d%n", ITERATIONS);
        System.out.printf("Minimum:    %.3f ms%n", min);
        System.out.printf("Maximum:    %.3f ms%n", max);
        System.out.printf("Average:    %.3f ms%n", avg);
        System.out.printf("Median:     %.3f ms%n", median);
        System.out.println("\n[Representative Value (Minimum)]: " + min + " ms");
    }

    private static boolean isSorted(int[] arr) {
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] < arr[i - 1]) return false;
        }
        return true;
    }

    private static double getMedian(double[] arr) {
        double[] sorted = arr.clone();
        Arrays.sort(sorted);
        int mid = sorted.length / 2;
        if (sorted.length % 2 == 0) {
            return (sorted[mid - 1] + sorted[mid]) / 2.0;
        } else {
            return sorted[mid];
        }
    }
}
