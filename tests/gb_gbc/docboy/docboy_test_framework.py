import glob
import shutil
import time
import os
import subprocess
from configparser import ConfigParser
from datetime import datetime

resultsDir = 'results\\'
ini = ConfigParser()
emulatorName = ''
emulatorDir = ''
testEmuDir = 'EmulatorToTest\\'
emulatorUnderTest = ''
romDir = ''
defaultTimeout = 5

EXPECTED_SHA1 = 'ab0e7bd1e1652db3995f554115d7e1b4b6783224'


def initializeFramework():
    global emulatorName, emulatorDir, testEmuDir, emulatorUnderTest, romDir, defaultTimeout

    scriptDir = os.path.dirname(os.path.abspath(__file__))
    ini.read(os.path.join(scriptDir, 'docboy_test_framework.ini'))

    emulatorName = ini.get('framework', 'emulator')
    emulatorDir = ini.get('framework', 'location')
    emuType = ini.get('framework', 'type')
    romDir = ini.get('framework', 'rom_dir')
    defaultTimeout = ini.getint('framework', 'default_timeout')

    os.makedirs(testEmuDir + emuType + "\\", exist_ok=True)
    shutil.copy2(emulatorDir, testEmuDir + emuType + "\\")

    emulatorUnderTest = testEmuDir + emuType + "\\" + emulatorName


def runDocboyTests():
    global emulatorUnderTest, romDir, defaultTimeout

    rom_pattern = os.path.join(romDir, "**", "*.gb")
    rom_files = glob.glob(rom_pattern, recursive=True)

    if not rom_files:
        print(f"No .gb files found in directory: {romDir}")
        return

    report_path = "docboy_test.report"
    report = open(report_path, "w")

    print(f"Found {len(rom_files)} ROM(s) to test. Executing...")

    for rom_path in rom_files:
        rom_name = os.path.basename(rom_path)

        print(f"Running: {rom_name} (Timeout: {defaultTimeout}s)")

        if os.path.exists("sha1.txt"):
            os.remove("sha1.txt")

        arg = [emulatorUnderTest, "--sha1", rom_path, "--timeout", str(defaultTimeout)]

        try:
            process = subprocess.Popen(arg)

            start_time = time.time()

            while time.time() - start_time < defaultTimeout + 2:
                if os.path.exists("sha1.txt"):
                    with open("sha1.txt", "r") as file:
                        actual_sha1 = file.read().strip().lower()

                    if actual_sha1 == EXPECTED_SHA1:
                        report.write(f"{rom_name}:Pass\n")
                        print(f"{rom_name}:Pass")
                    else:
                        report.write(f"{rom_name}:Fail\n")
                        print(f"{rom_name}:Fail")
                        print(f"  Expected SHA1: {EXPECTED_SHA1}")
                        print(f"  Actual SHA1:   {actual_sha1}")

                    process.kill()
                    process.wait()
                    break

                if process.poll() is not None:
                    report.write(f"{rom_name}:Fail\n")
                    print(f"{rom_name}:Fail - Process exited before SHA1")
                    break

                time.sleep(0.05)
            else:
                process.kill()
                process.wait()
                report.write(f"{rom_name}:Fail\n")
                print(f"{rom_name}:Fail - Timeout")

        except Exception as e:
            if process.poll() is None:
                process.kill()
                process.wait()

            report.write(f"{rom_name}:Fail\n")
            print(f"{rom_name}:Fail - Error: {str(e)}")

        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
            report.write(f"{rom_name}:Fail\n")
            print(f"{rom_name}:Fail - Timeout")

        except Exception as e:
            report.write(f"{rom_name}:Fail\n")
            print(f"{rom_name}:Fail - Error: {str(e)}")

    report.close()

    if os.path.exists("sha1.txt"):
        os.remove("sha1.txt")

    today = datetime.now()
    dest_dir = resultsDir + emulatorName.split('.')[0] + "_docboy" + today.strftime("_%m%d%Y%H%M%S") + "\\"
    os.makedirs(dest_dir, exist_ok=True)

    if os.path.exists(report_path):
        shutil.move(report_path, dest_dir)

    print(f"Tests complete. Results saved to {dest_dir}")


if __name__ == '__main__':
    print('Starting Masqerade\'s Docboy automated test framework...')
    initializeFramework()
    print(f'Emulator under test: {emulatorName}')
    runDocboyTests()