import java.util.Arrays;
import java.util.Random;

/**
 * Benchmark comparing JDK 6 style Single-Pivot Quicksort vs JDK 7+ Dual-Pivot Quicksort
 */
public class JDK6SortBenchmark {

    // Threshold for switching to insertion sort (same as JDK 6)
    private static final int INSERTION_SORT_THRESHOLD = 7;

    public static void main(String[] args) {
        final int SIZE = 10_000_000;
        final int ITERATIONS = 30;

        // Generate random data
        Random random = new Random(42);
        int[] original = new int[SIZE];
        for (int i = 0; i < SIZE; i++) {
            original[i] = random.nextInt();
        }

        // Warmup JIT
        System.out.println("Warming up JIT...");
        for (int i = 0; i < 5; i++) {
            int[] warmup = original.clone();
            jdk6Sort(warmup, 0, warmup.length);
        }

        // Benchmark JDK 6 Single-Pivot Quicksort
        System.out.println("\n=== JDK 6 Style Single-Pivot Quicksort ===");
        System.out.println("Running " + ITERATIONS + " iterations on " + SIZE + " integers...");
        double[] times = new double[ITERATIONS];

        for (int i = 0; i < ITERATIONS; i++) {
            int[] data = original.clone();

            long start = System.nanoTime();
            jdk6Sort(data, 0, data.length);
            long end = System.nanoTime();

            times[i] = (end - start) / 1_000_000.0;

            if (i == 0 && !isSorted(data)) {
                System.err.println("ERROR: Array not sorted!");
                System.exit(1);
            }

            System.out.printf("  Iteration %2d: %.3f ms%n", i + 1, times[i]);
        }

        double min = Arrays.stream(times).min().orElse(0);
        double max = Arrays.stream(times).max().orElse(0);
        double avg = Arrays.stream(times).average().orElse(0);

        System.out.println("\n=== JDK 6 Single-Pivot Results ===");
        System.out.printf("Minimum:    %.3f ms%n", min);
        System.out.printf("Maximum:    %.3f ms%n", max);
        System.out.printf("Average:    %.3f ms%n", avg);
        System.out.println("[Representative Value]: " + min + " ms");

        // Now benchmark JDK 7+ Dual-Pivot for comparison
        System.out.println("\n=== JDK 7+ Dual-Pivot Quicksort (Arrays.sort) ===");
        for (int i = 0; i < 5; i++) {
            int[] warmup = original.clone();
            Arrays.sort(warmup);
        }

        double[] times2 = new double[ITERATIONS];
        for (int i = 0; i < ITERATIONS; i++) {
            int[] data = original.clone();

            long start = System.nanoTime();
            Arrays.sort(data);
            long end = System.nanoTime();

            times2[i] = (end - start) / 1_000_000.0;
            System.out.printf("  Iteration %2d: %.3f ms%n", i + 1, times2[i]);
        }

        double min2 = Arrays.stream(times2).min().orElse(0);
        double max2 = Arrays.stream(times2).max().orElse(0);
        double avg2 = Arrays.stream(times2).average().orElse(0);

        System.out.println("\n=== JDK 7+ Dual-Pivot Results ===");
        System.out.printf("Minimum:    %.3f ms%n", min2);
        System.out.printf("Maximum:    %.3f ms%n", max2);
        System.out.printf("Average:    %.3f ms%n", avg2);
        System.out.println("[Representative Value]: " + min2 + " ms");

        // Summary
        System.out.println("\n========== COMPARISON ==========");
        System.out.printf("JDK 6 Single-Pivot: %.3f ms%n", min);
        System.out.printf("JDK 7+ Dual-Pivot:  %.3f ms%n", min2);
        System.out.printf("Speedup (Dual/Single): %.2fx%n", min / min2);
    }

    /**
     * JDK 6 style single-pivot quicksort implementation.
     * Based on the actual JDK 6 source code from java.util.Arrays.
     */
    private static void jdk6Sort(int[] a, int off, int len) {
        // Insertion sort on smallest arrays
        if (len < INSERTION_SORT_THRESHOLD) {
            for (int i = off; i < len + off; i++) {
                for (int j = i; j > off && a[j - 1] > a[j]; j--) {
                    swap(a, j, j - 1);
                }
            }
            return;
        }

        // Choose a partition element, v
        int m = off + (len >> 1); // Small arrays, middle element

        if (len > INSERTION_SORT_THRESHOLD) {
            int l = off;
            int n = off + len - 1;
            if (len > 40) { // Big arrays, pseudomedian of 9
                int s = len / 8;
                l = med3(a, l, l + s, l + 2 * s);
                m = med3(a, m - s, m, m + s);
                n = med3(a, n - 2 * s, n - s, n);
            }
            m = med3(a, l, m, n); // Mid-size, med of 3
        }
        int v = a[m];

        // Establish Invariant: v* (<v)* (>v)* v*
        int a1 = off, b = a1, c = off + len - 1, d = c;
        while (true) {
            while (b <= c && a[b] <= v) {
                if (a[b] == v)
                    swap(a, a1++, b);
                b++;
            }
            while (c >= b && a[c] >= v) {
                if (a[c] == v)
                    swap(a, c, d--);
                c--;
            }
            if (b > c)
                break;
            swap(a, b++, c--);
        }

        // Swap partition elements back to middle
        int s, n = off + len;
        s = Math.min(a1 - off, b - a1);
        vecswap(a, off, b - s, s);
        s = Math.min(d - c, n - d - 1);
        vecswap(a, b, n - s, s);

        // Recursively sort non-partition-elements
        if ((s = b - a1) > 1)
            jdk6Sort(a, off, s);
        if ((s = d - c) > 1)
            jdk6Sort(a, n - s, s);
    }

    private static void swap(int[] a, int i, int j) {
        int t = a[i];
        a[i] = a[j];
        a[j] = t;
    }

    private static void vecswap(int[] a, int b, int c, int n) {
        for (int i = 0; i < n; i++, b++, c++)
            swap(a, b, c);
    }

    private static int med3(int[] a, int b, int c, int d) {
        return (a[b] < a[c] ?
                (a[c] < a[d] ? c : a[b] < a[d] ? d : b) :
                (a[c] > a[d] ? c : a[b] > a[d] ? d : b));
    }

    private static boolean isSorted(int[] arr) {
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] < arr[i - 1]) return false;
        }
        return true;
    }
}
