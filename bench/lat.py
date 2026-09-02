#!/usr/bin/env python3
# Per-operation latency, read off e's OWN top-bar timer (ns resolution, no harness overhead):
# it reports each keystroke's path from bytes-arrived to frame-composed.
#
#   bench/lat.py                    # arrow key on e.c
#   bench/lat.py FILE OP REPS       # OP = arrow | type | page
import os, sys, pty, select, struct, fcntl, termios, time, statistics, pyte

D = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COLS, ROWS = 132, 40
OPS = {"arrow": b"\x1b[B", "type": b"x", "page": b"\x1b[6~"}

def main():
    f    = sys.argv[1] if len(sys.argv) > 1 else os.path.join(D, "e.c")
    op   = sys.argv[2] if len(sys.argv) > 2 else "arrow"
    reps = int(sys.argv[3]) if len(sys.argv) > 3 else 25
    pid, fd = pty.fork()
    if pid == 0:
        fcntl.ioctl(0, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
        os.execv(os.path.join(D, "e"), ["e", f])
    sc = pyte.Screen(COLS, ROWS); st = pyte.ByteStream(sc)
    def pump(t):
        end = time.time() + t
        while time.time() < end:
            r, _, _ = select.select([fd], [], [], max(0, end - time.time()))
            if not r: break
            try: st.feed(os.read(fd, 65536))
            except OSError: break
    read = lambda: sc.display[0][COLS - 53:COLS - 45].strip()
    pump(3.0); boot = read()
    vals = []
    for _ in range(reps):
        os.write(fd, OPS[op]); pump(0.05)
        try: vals.append(float(read()[:-2]))
        except ValueError: pass
    try:
        os.write(fd, b"\x1b\x1b"); pump(0.4); os.kill(pid, 9); os.waitpid(pid, 0)
    except Exception: pass
    print(f"{os.path.basename(f)} ({sum(1 for _ in open(f, errors='ignore')):,} lines)  startup {boot}"
          f"   {op}: median {statistics.median(vals):.4f}ms  max {max(vals):.4f}ms  n={len(vals)}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
