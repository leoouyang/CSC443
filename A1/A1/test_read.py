import subprocess

FILENAME = "test"
BLOCK_SIZES = [100, 256, 1024, 4096, 16384, 32768, 100000, 300000, 1000000, 3000000]

for block_size in BLOCK_SIZES:
    subprocess.call(["./get_histogram", FILENAME, str(block_size)])