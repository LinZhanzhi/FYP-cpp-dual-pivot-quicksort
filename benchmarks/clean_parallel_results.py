import csv
import os

# Define paths
full_csv_path = "benchmarks/results/aggregate/summary_full.csv"
rep_csv_path = "benchmarks/results/aggregate/summary_representative.csv"

def clean_csv(file_path):
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return

    print(f"Processing {file_path}...")
    try:
        rows = []
        header = None
        removed_count = 0
        
        with open(file_path, 'r', newline='') as f:
            reader = csv.reader(f)
            try:
                header = next(reader)
                rows.append(header)
                # Find Algorithm index
                algo_idx = header.index("Algorithm")
                
                for row in reader:
                    if row[algo_idx].startswith("dual_pivot_parallel"):
                        removed_count += 1
                    else:
                        rows.append(row)
            except StopIteration:
                pass # Empty file

        print(f"Removed {removed_count} rows from {file_path}")
        
        with open(file_path, 'w', newline='') as f:
            writer = csv.writer(f)
            writer.writerows(rows)
            
        print(f"Saved cleaned file to {file_path}")
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

if __name__ == "__main__":
    clean_csv(full_csv_path)
    clean_csv(rep_csv_path)
