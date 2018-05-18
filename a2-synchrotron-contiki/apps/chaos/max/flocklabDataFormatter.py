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

#def format_test_suite_name(name):
#    return f"{name}_{datetime.datetime.now():%Y-%m-%d_%H:%M:%S}"

outputs = {}


def csv_format_header_round_log(raw):
    column_labels = []
    parts = raw.split(',')
    for part in parts:
        column_labels.append(part.split(':')[0])

    return " ".join(column_labels)


def csv_format_round_log(raw):
    column_labels = []
    parts = raw.split(',')
    for part in parts:
        column_labels.append(part.split(':')[1])

    return " ".join(column_labels)

logpath = "log/"

roundLogPath = logpath + "round"
maxLogPath   = logpath + "max"
errorLogPath = logpath + "error"
rawLogPath   = logpath + "raw"

round = "round"
isFirstRoundPrint = "isFirstRoundPrint"
max = "max"
isFirstMaxPrint = "isFirstMaxPrint"
error = "error"
raw = "raw"

def asd(test_name, log_row):

    node_id = log_row[0]
    msg = log_row[1]
    # Has the output files been created.
    if node_id not in outputs:
        outputs[node_id] = {}

        outputs[node_id][round] = open(f"{test_name}/flocklab/{roundLogPath}/log_{node_id}.txt", "x")
        outputs[node_id][isFirstRoundPrint] = True

        outputs[node_id][max] = open(f"{test_name}/flocklab/{maxLogPath}/log_{node_id}.txt", "x")
        outputs[node_id][isFirstMaxPrint] = True

        outputs[node_id][error] = open(f"{test_name}/flocklab/{errorLogPath}/log_{node_id}.txt", "x")

        outputs[node_id][raw] = open(f"{test_name}/flocklab/{rawLogPath}/log_{node_id}.txt", "x")

    outputs[node_id][raw].write(msg + "\n")

    try:
        topic = msg[0:msg.index(' ')]
        raw_msg = msg[msg.index(' '):]
    except:
        print("helo")
        print(msg)

    if topic == "cluster_res:":
        if outputs[node_id][isFirstMaxPrint]:
            outputs[node_id][isFirstMaxPrint] = False
            formatted_header = csv_format_header_round_log(raw_msg)
            outputs[node_id][max].write(formatted_header + "\n")

        formatted_row = csv_format_round_log(raw_msg)
        outputs[node_id][max].write(formatted_row + "\n")

    elif topic == "chaos_round_report:":
        if outputs[node_id][isFirstRoundPrint]:
            outputs[node_id][isFirstRoundPrint] = False
            formatted_header = csv_format_header_round_log(raw_msg)
            outputs[node_id][round].write(formatted_header + "\n")

        formatted_row = csv_format_round_log(raw_msg)
        outputs[node_id][round].write(formatted_row + "\n")


def pre_process_log_row(log_row):
    parts = log_row.split(',')
    return parts[2], ",".join(parts[4:]).strip()


def create_folder_structure(test_name):
    os.makedirs(f"{test_name}/simulation_files")
    os.makedirs(f"{test_name}/flocklab/{roundLogPath}")
    os.makedirs(f"{test_name}/flocklab/{maxLogPath}")
    os.makedirs(f"{test_name}/flocklab/{errorLogPath}")
    os.makedirs(f"{test_name}/flocklab/{rawLogPath}")


def copy_flocklab_simulation_file(test_name):
    print(f"{test_name}/simulation_files/flocklab.csc")
    shutil.copy("flocklab.csc", f"{test_name}/simulation_files/flocklab.csc")

def main(args):
    parser=argparse.ArgumentParser()
    parser.add_argument('test_name', help='Put a name on the data being extracted')
    parser.add_argument('file', help='Path to the csv file with flocklab serial data.')
    args=parser.parse_args()

    #test_suite_name = 'dev' if args.name == 'dev' else format_test_suite_name(args.name)

    lines = open(args.file, "r").readlines()
    lines_count = len(lines)

    create_folder_structure(args.test_name)
    copy_flocklab_simulation_file(args.test_name)
    lines = enumerate(lines[1:])
    for i, row in lines:
        if i % 5000 == 0:
            print(i)
        row = pre_process_log_row(row)
        if row[1] is not '\0':
            asd(args.test_name, row)

    for ids in outputs:
        outputs[ids][max].close()
        outputs[ids][round].close()
        outputs[ids][error].close()
        outputs[ids][raw].close()
    # with open(
    #             os.path.join(test_suite_folder, LOCAL_TEST_INFORMATION_FILE),
    #             "w"
    #         ) as info:
    #     info.write(information)


if __name__ == '__main__':
    main(sys.argv)
