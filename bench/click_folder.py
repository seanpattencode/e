#!/usr/bin/env python3
"""
External timing: measures click-on-folder to render complete.
No editor instrumentation - pure external measurement.
"""

import subprocess
import time
import os
import pty
import select

def time_folder_click():
    master, slave = pty.openpty()
    os.set_blocking(master, False)

    # Start editor without args (directory mode)
    p = subprocess.Popen(
        ['/home/seanpatten/projects/e/e'],
        stdin=slave, stdout=slave, stderr=slave,
        cwd='/home/seanpatten/projects/e'
    )

    # Wait for initial render and drain
    time.sleep(0.15)
    try:
        while True:
            os.read(master, 10000)
    except:
        pass

    # Click on row 1, col 1 (first entry - docs/)
    # SGR format: \e[<btn;col;rowM (press) then \e[<btn;col;rowm (release)
    click = b'\x1b[<0;1;1M\x1b[<0;1;1m'

    # Measure from click send to first response byte
    start = time.perf_counter()
    os.write(master, click)

    # Wait for any output (screen update)
    os.set_blocking(master, True)
    select.select([master], [], [], 2)
    os.read(master, 1)  # First byte received
    end = time.perf_counter()

    elapsed_ms = (end - start) * 1000

    # Cleanup
    try:
        os.set_blocking(master, False)
        os.write(master, b'\x11')  # C-q to quit
    except:
        pass
    p.kill()
    p.wait()
    os.close(master)
    os.close(slave)

    return elapsed_ms


if __name__ == '__main__':
    print("Click-to-render timing (external)")
    print("=" * 40)

    times = []
    for i in range(10):
        t = time_folder_click()
        times.append(t)
        print(f"Run {i+1:2d}: {t:.2f}ms")

    print("-" * 40)
    print(f"Avg: {sum(times)/len(times):.2f}ms")
    print(f"Min: {min(times):.2f}ms")
    print(f"Max: {max(times):.2f}ms")
