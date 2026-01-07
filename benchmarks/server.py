import http.server
import socketserver
import json
import threading
import os
import sys
import time
import datetime
import subprocess
import glob
import csv

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import benchmark_manager

PORT = 8000
status_lock = threading.Lock()
runner_status = {"is_running": False, "current": 0, "total": 0, "message": ""}
class BenchmarkHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == "/":
            self.path = "index.html"
            return http.server.SimpleHTTPRequestHandler.do_GET(self)
        if self.path == "/api/config":
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            config = {"algorithms": benchmark_manager.ALGORITHMS, "types": benchmark_manager.TYPES, "patterns": benchmark_manager.PATTERNS, "sizes": benchmark_manager.SIZES}
            self.wfile.write(json.dumps(config).encode())
            return
        if self.path == "/api/testbeds":
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            aggregate_dir = benchmark_manager.AGGREGATE_DIR
            testbeds = []
            if os.path.exists(aggregate_dir):
                for name in os.listdir(aggregate_dir):
                    if os.path.isdir(os.path.join(aggregate_dir, name)):
                        testbeds.append(name)
            self.wfile.write(json.dumps(testbeds).encode())
            return
        if self.path.startswith("/api/results"):
            from urllib.parse import urlparse, parse_qs
            query = parse_qs(urlparse(self.path).query)
            testbed = query.get("testbed", [None])[0]

            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            results = []

            # Mapping from Display Names (CSV) to Config Keys (benchmark_manager)
            pattern_map = {
                "Random": "RANDOM",
                "Nearly Sorted": "NEARLY_SORTED",
                "Reverse Sorted": "REVERSE_SORTED",
                "Organ Pipe": "ORGAN_PIPE",
                "Sawtooth": "SAWTOOTH",
                "10% Unique": "MANY_DUPLICATES_10",
                "50% Unique": "MANY_DUPLICATES_50",
                "90% Unique": "MANY_DUPLICATES_90"
            }

            if testbed and testbed != "null":
                # Serve from aggregate file
                csv_path = os.path.join(benchmark_manager.AGGREGATE_DIR, testbed, "summary_representative.csv")
                if os.path.exists(csv_path):
                    try:
                        with open(csv_path, 'r') as f:
                            reader = csv.DictReader(f)
                            for row in reader:
                                raw_pattern = row['Pattern']
                                normalized_pattern = pattern_map.get(raw_pattern, raw_pattern)

                                results.append({
                                    "id": f"{row['Algorithm']}_{row['Type']}_{normalized_pattern}_{row['Size']}",
                                    "algo": row['Algorithm'],
                                    "type": row['Type'],
                                    "pattern": normalized_pattern,
                                    "size": int(row['Size']),
                                    "done": True,
                                    "timestamp": "", # Not available in aggregate
                                    "runtime": row['Time(ms)'] + " ms"
                                })
                    except Exception as e:
                        print(f"Error reading aggregate CSV: {e}")
            else:
                import itertools
                combinations = list(itertools.product(benchmark_manager.ALGORITHMS, benchmark_manager.TYPES, benchmark_manager.PATTERNS, benchmark_manager.SIZES))
                for algo, type_, pattern, size in combinations:
                    filename = benchmark_manager.get_output_filename(algo, type_, pattern, size)
                    done = os.path.exists(filename)
                    timestamp = ""
                    runtime = "-"
                    if done:
                        mtime = os.path.getmtime(filename)
                        timestamp = datetime.datetime.fromtimestamp(mtime).strftime('%Y-%m-%d %H:%M:%S')
                        try:
                            with open(filename, 'r') as f:
                                lines = f.readlines()
                                if len(lines) > 1:
                                    last_line = lines[-1].strip()
                                    parts = last_line.split(',')
                                    if len(parts) >= 5:
                                        runtime = parts[-1] + " ms"
                        except Exception:
                            pass
                    results.append({"id": f"{algo}_{type_}_{pattern}_{size}", "algo": algo, "type": type_, "pattern": pattern, "size": size, "done": done, "timestamp": timestamp, "runtime": runtime})
            self.wfile.write(json.dumps(results).encode())
            return

        if self.path.startswith("/api/details"):
            from urllib.parse import urlparse, parse_qs
            query = parse_qs(urlparse(self.path).query)
            test_id = query.get("id", [None])[0]

            if test_id:
                # ID format: algo_type_pattern_size
                try:
                    # Reconstruct filename from ID
                    # This is a bit fragile if ID format changes, but consistent with current logic
                    parts = test_id.split('_')
                    # Size is the last part, Pattern is the one before it...
                    # Wait, pattern can have underscores (MANY_DUPLICATES_10).
                    # Algo can have underscores (std_sort).
                    # Type is simple.
                    # Let's rely on the fact that we can reconstruct it if we know the components.
                    # Actually, it's safer to search for the file or pass components in query.
                    # But let's try to parse.
                    # Better: Client sends components.
                    pass
                except Exception:
                    pass

            # Alternative: Client sends components in query string
            algo = query.get("algo", [None])[0]
            type_ = query.get("type", [None])[0]
            pattern = query.get("pattern", [None])[0]
            size = query.get("size", [None])[0]
            testbed = query.get("testbed", [None])[0]

            if algo and type_ and pattern and size:
                if testbed and testbed != "null":
                    # Read from aggregate full summary
                    csv_path = os.path.join(benchmark_manager.AGGREGATE_DIR, testbed, "summary_full.csv")
                    if os.path.exists(csv_path):
                        try:
                            # Parse full summary and filter
                            output_lines = ["Algorithm,Type,Pattern,Size,Iteration,Time(ms)"]

                            # Same mapping needed
                            pattern_map = {
                                "Random": "RANDOM",
                                "Nearly Sorted": "NEARLY_SORTED",
                                "Reverse Sorted": "REVERSE_SORTED",
                                "Organ Pipe": "ORGAN_PIPE",
                                "Sawtooth": "SAWTOOTH",
                                "10% Unique": "MANY_DUPLICATES_10",
                                "50% Unique": "MANY_DUPLICATES_50",
                                "90% Unique": "MANY_DUPLICATES_90"
                            }

                            with open(csv_path, 'r') as f:
                                reader = csv.DictReader(f)
                                for row in reader:
                                    raw_pattern = row['Pattern']
                                    norm_pattern = pattern_map.get(raw_pattern, raw_pattern)

                                    if (row['Algorithm'] == algo and
                                        row['Type'] == type_ and
                                        norm_pattern == pattern and
                                        row['Size'] == size):
                                        output_lines.append(f"{row['Algorithm']},{row['Type']},{row['Pattern']},{row['Size']},{row['Iteration']},{row['Time(ms)']}")

                            self.send_response(200)
                            self.send_header("Content-type", "text/csv")
                            self.end_headers()
                            self.wfile.write("\n".join(output_lines).encode())
                            return
                        except Exception as e:
                            self.send_response(500)
                            self.wfile.write(str(e).encode())
                            return
                    else:
                        self.send_response(404)
                        self.wfile.write(b"Aggregate file not found")
                        return

                filename = benchmark_manager.get_output_filename(algo, type_, pattern, int(size))
                if os.path.exists(filename):
                    try:
                        with open(filename, 'r') as f:
                            content = f.read()
                        self.send_response(200)
                        self.send_header("Content-type", "text/csv")
                        self.end_headers()
                        self.wfile.write(content.encode())
                        return
                    except Exception as e:
                        self.send_response(500)
                        self.wfile.write(str(e).encode())
                        return

            self.send_response(404)
            self.wfile.write(b"Not found")
            return
        if self.path == "/api/status":
            self.send_response(200)
            self.send_header("Content-type", "application/json")
            self.end_headers()
            with status_lock:
                self.wfile.write(json.dumps(runner_status).encode())
            return
        return http.server.SimpleHTTPRequestHandler.do_GET(self)
    def do_POST(self):
        if self.path == "/api/run":
            content_length = int(self.headers["Content-Length"])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data)
            tests = data.get("tests", [])
            with status_lock:
                if runner_status["is_running"]:
                    self.send_response(409)
                    self.end_headers()
                    self.wfile.write(b"Already running")
                    return
                runner_status["is_running"] = True
                runner_status["current"] = 0
                runner_status["total"] = len(tests)
                runner_status["message"] = "Starting..."
            thread = threading.Thread(target=run_tests_background, args=(tests,), daemon=True)
            thread.start()
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"Started")
            return
        if self.path == "/api/playground/run":
            content_length = int(self.headers["Content-Length"])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data)
            size = data.get("size")
            pattern = data.get("pattern")
            type_ = data.get("type", "int")

            if not size or not pattern:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b"Missing size or pattern")
                return

            # Run the interactive runner
            runner_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "build", "interactive_runner")

            action = data.get("action", "run") # run, generate, sort

            cmd = [runner_path]

            if action == "generate":
                cmd.extend(["--generate", "--size", str(size), "--pattern", pattern, "--type", type_])
            elif action == "sort":
                input_data = data.get("data")
                if not input_data:
                    self.send_response(400)
                    self.end_headers()
                    self.wfile.write(b"Missing data for sort")
                    return

                # Use temp file to avoid argument length limits
                import tempfile

                # Ensure data is a string
                data_str = str(input_data)
                # Clean up brackets if present (though frontend should send string now)
                data_str = data_str.replace("[", "").replace("]", "")

                with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as tmp:
                    tmp.write(data_str)
                    tmp_path = tmp.name

                try:
                    cmd.extend(["--sort", "--data-file", tmp_path, "--type", type_])
                    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
                    self.send_response(200)
                    self.send_header("Content-type", "application/json")
                    self.end_headers()
                    self.wfile.write(result.stdout.encode())
                except subprocess.CalledProcessError as e:
                    self.send_response(500)
                    self.end_headers()
                    self.wfile.write(f"Error running benchmark: {e.stderr}".encode())
                except Exception as e:
                    self.send_response(500)
                    self.end_headers()
                    self.wfile.write(f"Error: {str(e)}".encode())
                finally:
                    if os.path.exists(tmp_path):
                        os.remove(tmp_path)
                return
            else:
                # Legacy/Default
                cmd.extend(["--size", str(size), "--pattern", pattern, "--type", type_])

            try:
                result = subprocess.run(cmd, capture_output=True, text=True, check=True)
                self.send_response(200)
                self.send_header("Content-type", "application/json")
                self.end_headers()
                self.wfile.write(result.stdout.encode())
            except subprocess.CalledProcessError as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(f"Error running benchmark: {e.stderr}".encode())
            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(f"Error: {str(e)}".encode())
            return
        if self.path == "/api/delete":
            content_length = int(self.headers["Content-Length"])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data)
            test = data.get("test", {})
            if test:
                filename = benchmark_manager.get_output_filename(test["algo"], test["type"], test["pattern"], int(test["size"]))
                if os.path.exists(filename):
                    try:
                        os.remove(filename)
                        self.send_response(200)
                        self.end_headers()
                        self.wfile.write(b"Deleted")
                    except Exception as e:
                        self.send_response(500)
                        self.end_headers()
                        self.wfile.write(f"Error deleting: {e}".encode())
                else:
                    self.send_response(404)
                    self.end_headers()
                    self.wfile.write(b"File not found")
            else:
                self.send_response(400)
                self.end_headers()
                self.wfile.write(b"Invalid request")
            return
def run_tests_background(tests):
    global runner_status
    try:
        for i, test in enumerate(tests):
            with status_lock:
                runner_status["current"] = i + 1
                runner_status["message"] = f"Running {test['algo']} {test['type']} {test['pattern']} {test['size']}"
            try:
                benchmark_manager.run_single_test(
                    test["algo"],
                    test["type"],
                    test["pattern"],
                    int(test["size"])
                )
            except Exception as e:
                print(f"Error running test {test}: {e}")
    finally:
        with status_lock:
            runner_status["is_running"] = False
            runner_status["message"] = "Idle"
            runner_status["current"] = 0
            runner_status["total"] = 0

if __name__ == "__main__":
    # Ensure we run from the directory where server.py and index.html are located
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    PORT = 8000
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), BenchmarkHandler) as httpd:
        print(f"Serving at http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped by user.")
