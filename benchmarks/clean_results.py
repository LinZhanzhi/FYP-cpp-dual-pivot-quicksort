import csv
import os

# Define the files relative to the workspace root
files_to_process = [
    'benchmarks/results/aggregate/summary_full.csv',
    'benchmarks/results/aggregate/summary_representative.csv'
]

def process_file(file_path):
    print(f"Processing {file_path}...")
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        return

    data_to_keep = []
    headers = []
    removed_count = 0

    # Read the file
    with open(file_path, 'r', newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        headers = reader.fieldnames

        if not headers:
            print(f"Warning: {file_path} has no headers.")
            return

        for row in reader:
            if 'Algorithm' in row:
                if row['Algorithm'].startswith('dual_pivot_parallel'):
                    removed_count += 1
                    continue
                data_to_keep.append(row)
            else:
                # If the Algorithm column is missing, keep the row
                data_to_keep.append(row)

    print(f"Removed {removed_count} rows.")
    print(f"Kept {len(data_to_keep)} rows.")

    # Write the file back
    with open(file_path, 'w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=headers)
        writer.writeheader()
        writer.writerows(data_to_keep)
    print(f"Finished processing {file_path}\n")

if __name__ == "__main__":
    # Detect workspace root based on this script's location
    # Script is at /home/lzz725/FYP/benchmarks/clean_results.py
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_root = os.path.dirname(script_dir) # Go up one level to FYP root

    for f in files_to_process:
        full_path = os.path.join(workspace_root, f)
        process_file(full_path)
