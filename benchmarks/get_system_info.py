import subprocess
import platform
import os
import re
import sys
import multiprocessing

def get_cpu_info_linux():
    info = {}
    try:
        # Try lscpu first as it provides a good summary
        result = subprocess.run(['lscpu'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        if result.returncode == 0:
            for line in result.stdout.splitlines():
                if ':' in line:
                    key, value = line.split(':', 1)
                    info[key.strip()] = value.strip()
    except Exception:
        pass

    # Fallback/Supplemental: /proc/cpuinfo for specific model if lscpu vague
    if 'Model name' not in info:
        try:
            with open('/proc/cpuinfo', 'r') as f:
                for line in f:
                    if 'model name' in line:
                        info['Model name'] = line.split(':', 1)[1].strip()
                        break
        except:
            pass

    return info

def get_ram_info_linux():
    total_mem_gb = 0
    try:
        with open('/proc/meminfo', 'r') as f:
            for line in f:
                if 'MemTotal' in line:
                    # MemTotal:       16301284 kB
                    parts = line.split()
                    kb = int(parts[1])
                    total_mem_gb = kb / 1024 / 1024
                    break
    except:
        pass
    return f"{total_mem_gb:.2f} GB"

def get_os_info():
    os_name = "Unknown"
    kernel = platform.release()

    # Check for WSL
    is_wsl = 'microsoft' in kernel.lower() or 'wsl' in kernel.lower()

    try:
        if os.path.exists('/etc/os-release'):
            with open('/etc/os-release') as f:
                for line in f:
                    if line.startswith('PRETTY_NAME='):
                        os_name = line.split('=', 1)[1].strip().strip('"')
                        break
    except:
        os_name = platform.system() + " " + platform.release()

    return os_name, kernel, is_wsl

def main():
    print("Gathering System Information...")
    print("-" * 40)

    # CPU
    cpu_info = get_cpu_info_linux()
    model = cpu_info.get('Model name', 'Unknown CPU')
    threads = multiprocessing.cpu_count()

    # RAM
    ram = get_ram_info_linux()

    # OS
    os_name, kernel, is_wsl = get_os_info()

    # Markdown Output for Report
    print("\n### System Specifications (Report Format)\n")
    print(f"**Host Machine**")
    if is_wsl:
        print(f"⚫ Environment: WSL2 (Windows Subsystem for Linux)")

    print(f"⚫ CPU: {model}")
    print(f"   - Logical Processors (Threads): {threads}")
    # Architecture usually implies detected by lscpu
    if 'Architecture' in cpu_info:
        print(f"   - Architecture: {cpu_info['Architecture']}")

    print(f"⚫ RAM: {ram}")
    print(f"⚫ Operating System: {os_name}")
    print(f"   - Kernel: {kernel}")

if __name__ == "__main__":
    main()
