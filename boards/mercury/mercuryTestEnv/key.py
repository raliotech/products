import sys
import subprocess
from subprocess import STDOUT
import time
import shlex
import argparse
import concurrent.futures
import threading

# ANSI Codes for screen manipulation
CLEAR_LINE = '\033[2K'
UP_LINE = '\033[A'

print_lock = threading.Lock()

port_line_map = {}
TOTAL_LINES = 0


def erase_flash(port):
    command = f"esptool -p {port} erase_flash"
    try:
        subprocess.run(shlex.split(command), check=True, text=True, stdout=subprocess.PIPE, stderr=STDOUT)
        with print_lock:
            print(f"✅ [{port}] Erase successful.")
    except subprocess.CalledProcessError as e:
        with print_lock:
            error_output = e.stdout if e.stdout else "No output"
            print(f"❌ [{port}] Error erasing flash (exit {e.returncode}): {error_output.strip()}")
    except FileNotFoundError:
        with print_lock:
            print(f"❌ [{port}] Error: esptool.py not found. Please ensure it is in your PATH.")


def terminal_print_progress(port, line, final_status=False):
    """
    Prints a line for a specific port by moving the cursor to the port's
    dedicated line, printing the update, and restoring the cursor position.
    """
    line_index = port_line_map[port]
    lines_up_to_move = TOTAL_LINES - line_index

    with print_lock:
        sys.stdout.write(f"\033[{lines_up_to_move}A")
        sys.stdout.write(f"{CLEAR_LINE}\r {port} {line}")

        if final_status:
            sys.stdout.write('\n')
            lines_down_to_move = lines_up_to_move - 1
        if not final_status:
            lines_down_to_move = lines_up_to_move
            # sys.stdout.write(f"\033[{lines_up_to_move}B")
        if lines_down_to_move > 0:
            sys.stdout.write(f"\033[{lines_down_to_move}B")
        sys.stdout.flush()


def write_flash(port, bin_file="./mercuryTest.ino.bin"):
    """
    Flashes the ESP8266. Uses Popen to stream the output and the
    terminal_print_progress function to update the dedicated line.
    """
    command = f"stdbuf -o L esptool.py -p {port} --chip esp8266 --before default-reset --after hard-reset write-flash --flash-mode dio 0x0 {bin_file}"
    command_list = shlex.split(command)

    try:
        # Start the process
        process = subprocess.Popen(
            command_list,
            stdout=subprocess.PIPE,
            stderr=STDOUT,  # Merge stderr into stdout
            text=True,
            bufsize=1  # Line buffering
        )

        # Initial status before reading output
        terminal_print_progress(port, "Starting...", final_status=False)

        # Read the output line by line
        while True:
            line = process.stdout.readline()
            if not line:
                break
            line = line.rstrip('\n')
            if "Writing" in line:
                terminal_print_progress(port, line, final_status=False)
        process.wait()

        # Print the final status
        if process.returncode == 0:
            final_line = "✅ Flash successful"
        else:
            final_line = "❌ Flash failed"
        terminal_print_progress(port, final_line, final_status=True)

    except FileNotFoundError:
        terminal_print_progress(port, "❌ Flash failed: esptool.py not found.", final_status=True)
    except Exception as e:
        terminal_print_progress(port, f"❌ Flash failed: An unexpected error occurred: {e}", final_status=True)


def main():
    parser = argparse.ArgumentParser(description="Script to control Mercury v3 device via esptool.")
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # Erase subparser
    erase_parser = subparsers.add_parser("erase", help="Erase flash memory on a device or all devices.")
    erase_parser.add_argument("port", help="The serial port (e.g., /dev/ttyUSB0) or 'all' for all devices.")

    # Flash subparser
    flash_parser = subparsers.add_parser("flash", help="Write flash memory on a device or all devices.")
    flash_parser.add_argument("port", help="The serial port (e.g., /dev/ttyUSB0) or 'all' for all devices.")

    args = parser.parse_args()

    all_ports = ['/dev/ttyUSB0', '/dev/ttyUSB1', '/dev/ttyUSB2', '/dev/ttyUSB3', '/dev/ttyUSB4']

    if args.command is None:
        parser.print_help()
        sys.exit(1)

    target_ports = all_ports if args.port == "all" else [args.port]
    global TOTAL_LINES
    TOTAL_LINES = len(target_ports)

    if args.command == "flash":
        global port_line_map
        for i, port in enumerate(target_ports):
            port_line_map[port] = i
            if i == len(target_ports) - 1:
                print(f"[{port}]")
            else:
                print(f"[{port}] \n", end="")

        with concurrent.futures.ThreadPoolExecutor(max_workers=len(target_ports)) as executor:
            list(executor.map(write_flash, target_ports))

    elif args.command == "erase":
        with concurrent.futures.ThreadPoolExecutor(max_workers=len(target_ports)) as executor:
            list(executor.map(erase_flash, target_ports))

    time.sleep(1)


if __name__ == "__main__":
    main()

