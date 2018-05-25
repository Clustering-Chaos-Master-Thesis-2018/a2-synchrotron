#!/usr/bin/env python3
import subprocess
import os
import shlex
import sys
import datetime
import concurrent.futures
import re
import xml.etree.ElementTree as ET
import shutil
import argparse
import csv

RUN_TEST_COMMAND = ["sh", "runTest"]
TEST_DIRECTORY = "tests"

SIMULATION_FILE_EXTENSION = ".csc"
SCRIPT_TAG = ".//script"
SCRIPT_FILE = os.path.join(TEST_DIRECTORY, "simulationScript.js")

LOCAL_SIMULATION_DIRECTORY = "simulation_files"

TEST_DIRECTORY_STRUCTURE = {
    "outputs": [
        os.path.join("", "log", "error"),
        os.path.join("", "log", "round"),
        os.path.join("", "log", "max"),
        os.path.join("", "log", "raw"),
        os.path.join("", "log", "power"),
    ]
}

LOCAL_LOG_DIRECTORY = "log"
LOCAL_COOJA_LOG_FILE = "cooja.log"
LOCAL_TEST_INFORMATION_FILE = "information.txt"
CSV_FILE_NAME = "parameters.csv"
# Time is in seconds.
# SIMULATION_TIMEOUT = sys.argv[2] if len(sys.argv) > 2 else 60

GET_COMMIT_HASH = ["git", "log", "-n 1", '--pretty=format:"%h"']


def get_simulation_files(simulation_folder):
    return [
        os.path.join(root, *directory, file)
        for root, directory, files in os.walk(simulation_folder)
        for file in files
        if file.endswith(SIMULATION_FILE_EXTENSION)
    ]


def create_test_suite_folder_structure(test_suite_name):
    test_suite_folder = os.path.join(TEST_DIRECTORY, test_suite_name)
    simulation_folder = os.path.join(test_suite_folder, LOCAL_SIMULATION_DIRECTORY)

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
    return (
        f'var logpath = "{os.path.join(base_path, os.path.splitext(file_name)[0], LOCAL_LOG_DIRECTORY)}/";\n'
    )


def create_timeout_function_call(time):
    return f"TIMEOUT({ str(int(time) * 1000) });\n"


def create_script_plugin_tree(root):
    plugin = ET.SubElement(root, "plugin")
    plugin.text = "org.contikios.cooja.plugins.ScriptRunner"
    plugin_config = ET.SubElement(plugin, "plugin_config")
    script_tag = ET.SubElement(plugin_config, "script")

    return script_tag


def create_local_simulation_files(test_suite_folder, output_folder, simulation_timeout):
    """ Inserts the simulation script into the csc files and saves
        the new csc files to the output_folder."""
    simulation_files = get_simulation_files(SIMULATION_DIRECTORY)
    simulation_files = sorted(simulation_files)
    simulation_script = ""
    local_files = []

    with open(SCRIPT_FILE) as file:
        simulation_script = file.read()

    if not RUN_ALL_SIMULATIONS:
        print("")
        for i, sim_file in enumerate(simulation_files):
            print(f"{i}: {sim_file}")
        tests = input(
            "Which simulations do you want to run? (e.g. '0,1,3', leave empty to run all)\n>"
        )
        if tests:
            test_numbers = list(map(int, tests.split(",")))
            simulation_files = [simulation_files[x] for x in test_numbers]

    for simulation_file in simulation_files:
        output_file_path = os.path.join(
            output_folder, os.path.basename(simulation_file)
        )
        tree = ET.parse(simulation_file)
        root = tree.getroot()

        firmware_tag = root.find(".//firmware")
        if firmware_tag is None:
            motetype_tag = root.find(".//motetype")
            firmware_tag = ET.SubElement(motetype_tag, "firmware")
            firmware_tag.set("EXPORT", "copy")

        firmware_tag.text = (
            f"[CONTIKI_DIR]/apps/chaos/max/{test_suite_folder}/max-app.sky"
        )

        script_tag = root.find(SCRIPT_TAG)
        if script_tag is None:
            script_tag = create_script_plugin_tree(root)

        log_path = create_log_path_variable(
            test_suite_folder, os.path.basename(simulation_file)
        )
        timeout_call = create_timeout_function_call(simulation_timeout)
        script_tag.text = "".join([log_path, timeout_call, simulation_script])

        local_files.append(output_file_path)
        tree.write(output_file_path)

    return local_files


def run_test(test_suite_folder, file, simulation_timeout):
    path = create_local_test_folder(test_suite_folder, file)
    # print("Running test: " + os.path.basename(file) + " for " + str(simulation_timeout) + " seconds... ", end="", flush=True)
    print(
        f"Running test: {os.path.basename(file)} for {str(simulation_timeout)} seconds... ",
        flush=True,
    )

    with open(os.path.join(path, LOCAL_COOJA_LOG_FILE), "w") as cooja_log:
        output = ""
        process = subprocess.Popen(
            RUN_TEST_COMMAND + [file],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )
        for line in process.stdout:
            output += line
            cooja_log.write(line)
            cooja_log.flush()

    process.wait()

    match = re.search(r"Duration: (\d+).+?", output)
    if match:
        return match.group(1)
    else:
        return None


def format_date(date):
    return f"{date:%Y-%m-%d_%H.%M.%S}"


def format_test_suite_name(name):
    return f"{name}_{format_date(datetime.datetime.now())}"


def run_make_command(make_command):
    if not make_command.startswith("make clean"):
        subprocess.check_call("make clean".split())
    subprocess.check_call(make_command.split())


def create_make_dictionary(make_command):
    # Map all param=value to tuples: (param, value)
    string_to_list = map(lambda param: tuple(param.split("=")), make_command.split())
    # Filter out all tuples which hade no values
    return dict(filter(lambda value: len(value) > 1, string_to_list))


def create_csv_file(file_name, make_command, test_suite_name, start_time):
    dictionary = {}
    dictionary["name"] = test_suite_name
    dictionary["start_time"] = start_time
    dictionary["end_time"] = format_date(datetime.datetime.now())
    dictionary = {**dictionary, **create_make_dictionary(make_command)}

    with open(file_name, "w") as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=dictionary.keys(), delimiter=" ")

        writer.writeheader()
        writer.writerow(dictionary)


def main(args):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "name",
        help='Name of the test, e.g. "50-nodes-comp-radius-1". A timestamp will be appended to this. If the name is "dev" no timestamp is appended.',
    )
    parser.add_argument(
        "build_command",
        help="Command used to build the .sky file. Make clean is run before building.",
    )
    parser.add_argument(
        "sim_time", help="How long time should be simulated. In seconds."
    )
    parser.add_argument(
        "--sim_folder",
        help='Path to folder with simulation files. Defaults to "tests/Simulations"',
        default=os.path.join(TEST_DIRECTORY, "Simulations"),
    )
    parser.add_argument(
        "--run_all",
        help="Run all tests in folder. Default is off.",
        action="store_true",
        default=False,
    )
    args = parser.parse_args()

    global SIMULATION_DIRECTORY
    SIMULATION_DIRECTORY = args.sim_folder
    global RUN_ALL_SIMULATIONS
    RUN_ALL_SIMULATIONS = args.run_all

    test_suite_name = "dev" if args.name == "dev" else format_test_suite_name(args.name)

    start_time = format_date(datetime.datetime.now())
    run_make_command(args.build_command)
    simulation_timeout = args.sim_time

    test_suite_folder, test_suite_simulation_folder = create_test_suite_folder_structure(
        test_suite_name
    )
    local_files = create_local_simulation_files(
        test_suite_folder, test_suite_simulation_folder, simulation_timeout
    )
    shutil.copyfile("max-app.sky", os.path.join(test_suite_folder, "max-app.sky"))
    print(
        "Running test suite: "
        + test_suite_name
        + " with "
        + str(len(local_files))
        + " test(s)"
    )

    test_statistics = ""
    # We can use a with statement to ensure threads are cleaned up promptly
    with concurrent.futures.ThreadPoolExecutor() as executor:
        # Start the load operations and mark each future with its URL
        future_to_test = {
            executor.submit(run_test, test_suite_folder, file, simulation_timeout): file
            for file in local_files
        }
        for test in concurrent.futures.as_completed(future_to_test):
            file = future_to_test[test]
            data = test.result()
            if data:
                test_statistics += os.path.basename(file) + " ran for: " + data + "ms\n"
                print("test %r ran for %sms" % (os.path.basename(file), data))
            else:
                print(
                    "Error occured in cooja running test: %r" % (os.path.basename(file))
                )

    commit_hash = subprocess.check_output(GET_COMMIT_HASH)
    information = f"""Name: {test_suite_name}
        Time: {datetime.datetime.now():%Y-%m-%d_%H:%M:%S}
        Timeout: {simulation_timeout} seconds
        Commit: {commit_hash}
        build_command: {args.build_command}
        #Tests: {len(local_files)}\n"""

    information += test_statistics

    with open(
        os.path.join(test_suite_folder, LOCAL_TEST_INFORMATION_FILE), "w"
    ) as info:
        info.write(information)

    create_csv_file(
        os.path.join(test_suite_folder, CSV_FILE_NAME),
        args.build_command,
        test_suite_name,
        start_time,
    )


if __name__ == "__main__":
    main(sys.argv)
