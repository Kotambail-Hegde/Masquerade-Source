import glob
import os
import shutil
import subprocess
import time
from configparser import ConfigParser
from datetime import datetime
from pathlib import Path

# Base directories and config
resultsDir = Path('results')
ini = ConfigParser()
emulatorName = ''
emulatorDir = ''
testEmuDir = Path('EmulatorToTest')
emulatorUnderTest = ''
romDir = ''
defaultTimeout = 5


def initializeFramework():
    global emulatorName, emulatorDir, testEmuDir, emulatorUnderTest, romDir, defaultTimeout

    scriptDir = Path(__file__).resolve().parent
    ini.read(scriptDir / 'docboy_test_framework.ini')

    emulatorName = ini.get('framework', 'emulator')
    emulatorDir = ini.get('framework', 'location')
    emuType = ini.get('framework', 'type')
    romDir = ini.get('framework', 'rom_dir')
    defaultTimeout = ini.getint('framework', 'default_timeout')

    target_dir = testEmuDir / emuType
    target_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy2(emulatorDir, target_dir)

    emulatorUnderTest = str(target_dir / emulatorName)


def kill_process_tree(proc):
    """Forcefully kills a process along with all its child processes."""
    if proc is None or proc.poll() is not None:
        return

    try:
        if os.name == 'nt':
            # Force kill the process tree on Windows
            subprocess.run(
                ['taskkill', '/F', '/T', '/PID', str(proc.pid)],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
        else:
            proc.kill()
            proc.wait(timeout=1)
    except Exception:
        pass


def runDocboyTests():
    global emulatorUnderTest, romDir, defaultTimeout

    # Grab both .gb and .gbc files
    gb_pattern = os.path.join(romDir, "**", "*.gb")
    gbc_pattern = os.path.join(romDir, "**", "*.gbc")
    rom_files = glob.glob(gb_pattern, recursive=True) + glob.glob(gbc_pattern, recursive=True)
    
    total_tests = len(rom_files)

    if total_tests == 0:
        print(f"No .gb or .gbc files found in directory: {romDir}")
        return

    report_path = "docboy_test.report"

    print(f"Found {total_tests} ROM(s) to test. Executing...")

    with open(report_path, "w") as report:
        for index, rom_path in enumerate(rom_files, start=1):
            rom_name = os.path.basename(rom_path)

            # Assign the correct expected SHA1 based on the file extension
            if rom_name.lower().endswith('.gbc'):
                # below is with color correction enabled, but we are now testing with it disabled
                # expected_sha1 = 'dfa84905a29a707a172985dd4f2895eb8b0d8e3b'
                expected_sha1 = '52a51ab9a3001c101f970e281cd52073813cb186'
            else:
                # below is with GEARBOY palette, but we are now testing with Black/White palette
                # expected_sha1 = 'ab0e7bd1e1652db3995f554115d7e1b4b6783224'
                expected_sha1 = '52a51ab9a3001c101f970e281cd52073813cb186'

            # Print the current count out of the total
            print(f"Running [{index}/{total_tests}]: {rom_name} (Timeout: {defaultTimeout}s)")

            if os.path.exists("sha1.txt"):
                try:
                    os.remove("sha1.txt")
                except OSError:
                    pass

            arg = [
                emulatorUnderTest,
                "--sha1",
                rom_path,
                "--timeout",
                str(defaultTimeout),
                "--headless"
            ]

            process = None
            try:
                process = subprocess.Popen(
                    arg,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.DEVNULL
                )

                start_time = time.time()
                handled = False

                while time.time() - start_time < defaultTimeout + 2:
                    if os.path.exists("sha1.txt"):
                        time.sleep(0.05)  # Brief delay to allow file flush to disk
                        
                        try:
                            with open("sha1.txt", "r") as file:
                                actual_sha1 = file.read().strip().lower()
                        except OSError:
                            time.sleep(0.1)
                            with open("sha1.txt", "r") as file:
                                actual_sha1 = file.read().strip().lower()

                        # Compare against the dynamically set expected_sha1
                        if actual_sha1 == expected_sha1:
                            report.write(f"{rom_name}:Pass\n")
                            print(f"{rom_name}:Pass")
                        else:
                            report.write(f"{rom_name}:Fail\n")
                            print(f"{rom_name}:Fail")
                            print(f"  Expected SHA1: {expected_sha1}")
                            print(f"  Actual SHA1:   {actual_sha1}")

                        kill_process_tree(process)
                        handled = True
                        break

                    if process.poll() is not None:
                        report.write(f"{rom_name}:Fail\n")
                        print(f"{rom_name}:Fail - Process exited before SHA1 output")
                        handled = True
                        break

                    time.sleep(0.05)

                if not handled:
                    kill_process_tree(process)
                    report.write(f"{rom_name}:Fail\n")
                    print(f"{rom_name}:Fail - Timeout")

            except Exception as e:
                kill_process_tree(process)
                report.write(f"{rom_name}:Fail\n")
                print(f"{rom_name}:Fail - Error: {str(e)}")

    if os.path.exists("sha1.txt"):
        try:
            os.remove("sha1.txt")
        except OSError:
            pass

    today = datetime.now()
    folder_name = f"{Path(emulatorName).stem}_docboy{today.strftime('_%m%d%Y%H%M%S')}"
    dest_dir = resultsDir / folder_name
    dest_dir.mkdir(parents=True, exist_ok=True)

    if os.path.exists(report_path):
        shutil.move(report_path, dest_dir / report_path)

    print(f"Tests complete. Results saved to {dest_dir}")


if __name__ == '__main__':
    print("Starting Masquerade's Docboy automated test framework...")
    initializeFramework()
    print(f"Emulator under test: {emulatorName}")
    runDocboyTests()