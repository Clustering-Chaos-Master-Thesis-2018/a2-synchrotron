#!/usr/bin/env python3
import subprocess
import os
import shlex
import sys
import datetime
import concurrent.futures
import re
import xml.etree.ElementTree as ET

RUN_TEST_COMMAND = ["./runTest"]
TEST_DIRECTORY = "tests"
GLOBAL_SIMULATION_DIRECTORY = TEST_DIRECTORY + "/Simulations"

TEST_FILE_EXTENSION = ".csc"
SCRIPT_TAG = ".//script"
SCRIPT_FILE = os.path.join(TEST_DIRECTORY, "simulationScript.js")

LOCAL_SIMULATION_DIRECTORY = "simulation_files"

TEST_DIRECTORY_STRUCTURE = {
    "outputs": ["log/error", "log/round", "log/power"]
}

LOCAL_LOG_DIRECTORY = "log"
LOCAL_ERROR_DIRECTORY = "error"
LOCAL_COOJA_LOG_FILE = "cooja.log"
LOCAL_TEST_INFORMATION_FILE = "information.txt"
# Time is in seconds.
SIMULATION_TIMEOUT = sys.argv[2] if len(sys.argv) > 2 else 60

GET_COMMIT_HASH = ["git", "log", "-n 1", "--pretty=format:\"%h\""]


def get_global_simulation_files(folder):
    return [os.path.join(root, *directory, file) for root, directory, files in os.walk(folder) for file in files if file.endswith(TEST_FILE_EXTENSION)]


def create_test_suite_folder_structure(test_suite_name):
    test_suite_folder = os.path.join(TEST_DIRECTORY, test_suite_name)
    simulation_folder = os.path.join(
        test_suite_folder, LOCAL_SIMULATION_DIRECTORY)

    os.makedirs(simulation_folder, exist_ok=True)

    return test_suite_folder, simulation_folder


def create_local_test_folder(test_suite_directory, simulation_file):
    folder_name = os.path.splitext(os.path.basename(simulation_file))[0]
    local_test_folder = os.path.join(test_suite_directory, folder_name)
    os.makedirs(local_test_folder, exist_ok=True)

    for output_folder in TEST_DIRECTORY_STRUCTURE["outputs"]:
        output_directory = os.path.join(local_test_folder, output_folder)
        os.makedirs(output_directory, exist_ok=True)

    return local_test_folder


def create_log_path_variable(base_path, file_name):
    return f'var logpath = "{os.path.join(base_path, os.path.splitext(file_name)[0], LOCAL_LOG_DIRECTORY)}/";\n'


def create_error_path_variable(base_path, file_name):
    return f'var errorpath = "{os.path.join(base_path, os.path.splitext(file_name)[0], LOCAL_ERROR_DIRECTORY)}/";\n'


def create_timeout_function_call(time):
    return f"TIMEOUT({ str(int(time) * 1000) });\n"


def create_script_plugin_tree(root):
    plugin = ET.SubElement(root, "plugin")
    plugin.text = "org.contikios.cooja.plugins.ScriptRunner"
    plugin_config = ET.SubElement(plugin, "plugin_config")
    script_tag = ET.SubElement(plugin_config, "script")

    return script_tag


def create_local_simulation_files(test_suite_folder, output_folder):
    """ Inserts the simulation script into the csc files and saves
        the new csc files to the output_folder."""
    simulation_files = get_global_simulation_files(GLOBAL_SIMULATION_DIRECTORY)
    simulation_script = ""
    local_files = []

    with open(SCRIPT_FILE) as file:
        simulation_script = file.read()

    for simulation_file in simulation_files:
        output_file_path = os.path.join(
            output_folder, os.path.basename(simulation_file))
        tree = ET.parse(simulation_file)
        root = tree.getroot()

        script_tag = root.find(SCRIPT_TAG)
        if script_tag is None:
            script_tag = create_script_plugin_tree(root)

        log_path = create_log_path_variable(
            test_suite_folder, os.path.basename(simulation_file))
        error_path = create_error_path_variable(
            test_suite_folder, os.path.basename(simulation_file))
        timeout_call = create_timeout_function_call(SIMULATION_TIMEOUT)
        script_tag.text = "".join(
            [log_path, error_path, timeout_call, simulation_script]
        )

        local_files.append(output_file_path)
        tree.write(output_file_path)

    return local_files


def run_test(test_suite_folder, file):
    path = create_local_test_folder(test_suite_folder, file)
    # print("Running test: " + os.path.basename(file) + " for " + str(SIMULATION_TIMEOUT) + " seconds... ", end="", flush=True)
    print(
        f"Running test: {os.path.basename(file)} for {str(SIMULATION_TIMEOUT)} seconds... ",
        end="", flush=True
    )

    with open(os.path.join(path, LOCAL_COOJA_LOG_FILE), "w") as cooja_log:
        output = ""
        process = subprocess.Popen(
            RUN_TEST_COMMAND + [file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True
        )
        for line in process.stdout:
            output += line
            cooja_log.write(line)
            cooja_log.flush()

    process.wait()

    match = re.search(r'Duration: (\d+).+?', output)
    if match:
        return match.group(1)
    else:
        return None


def main(args):
    commit_hash = subprocess.check_output(GET_COMMIT_HASH)
    name = (f"{args[1]}_" if len(args) > 1 else "")
    test_suite_name = f"{name}{datetime.datetime.now():%Y-%m-%d_%H:%M:%S}"

    if len(args) > 1 and args[1] == "dev":
        test_suite_name = "dev"

    test_suite_folder, simulation_folder = create_test_suite_folder_structure(
        test_suite_name)
    local_files = create_local_simulation_files(
        test_suite_folder, simulation_folder)
    print("Running test suite: " + test_suite_name +
          " with " + str(len(local_files)) + " test(s)")

    test_statistics = ""
    # We can use a with statement to ensure threads are cleaned up promptly
    with concurrent.futures.ThreadPoolExecutor() as executor:
            # Start the load operations and mark each future with its URL
        future_to_test = {executor.submit(
            run_test, test_suite_folder, file): file for file in local_files}
        for test in concurrent.futures.as_completed(future_to_test):
            file = future_to_test[test]
            data = test.result()
            if data:
                test_statistics += os.path.basename(file) + \
                    " ran for: " + data + "ms\n"
                print('test %r ran for %sms' % (os.path.basename(file), data))
            else:
                print("Error occured in cooja running test: %r" %
                      (os.path.basename(file)))

    information = f"""Name: {test_suite_name}
        Time: {datetime.datetime.now():%Y-%m-%d_%H:%M:%S}
        Timeout: {SIMULATION_TIMEOUT} seconds
        Commit: {commit_hash}
        #Tests: {len(local_files)}\n"""

    information += test_statistics

    with open(
                os.path.join(test_suite_folder, LOCAL_TEST_INFORMATION_FILE),
                "w"
            ) as info:
        info.write(information)


if __name__ == '__main__':
    main(sys.argv)
