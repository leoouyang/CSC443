#!/usr/bin/python
import shutil
import subprocess
import os

if __name__ == "__main__":
    page_size_list = ["128","1024","2048","8192","16384","65536","262144","524288","1048576","3145728"]
    csv_total_tuple = "100000"  # number of tuples
    csv_file = "test.csv"
    searchHeapFile = "10"
    returnHeapFile = "20"
    start = "A"
    end = "B"

    if csv_file not in os.listdir("."):
        print("Creating test.csv...")
        subprocess.call(["./mkcsv.py", csv_file, csv_total_tuple])

    print("Testing Column Store Heap File...")
    for page_size in page_size_list:
        print("======================== {} ========================".format(page_size))
        for _ in range(0, 3):
            subprocess.call(["./csv2colstore", csv_file, "columnDir", page_size])
            print("====================================================")
            print("Testing Select2...")
            subprocess.call(["./select2", "columnDir",searchHeapFile,start,end,page_size])
            print("====================================================")
            print("Testing Select3...")
            subprocess.call(["./select3", "columnDir",searchHeapFile,returnHeapFile,start,end,page_size])
            shutil.rmtree("columnDir")
