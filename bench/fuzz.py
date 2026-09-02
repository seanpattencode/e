#!/usr/bin/env python3
# Differential edit fuzz: drive two e binaries with the SAME random key sequences and compare the
# files they save. Screen parity proves the rendering matches; this proves the editing does.
#
#   bench/fuzz.py                   # committed binary (git show HEAD:e) vs ./e, 16 sequences
#   bench/fuzz.py SEED N OLD NEW
# exits non-zero on any mismatch.
import os, sys, pty, select, time, random, subprocess, tempfile

D = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
KEYS = [b"a", b"b", b" ", b"\t", b"\r", b"\x7f", b"\x7f", b"\x1b[3~", b"\x0b", b"\x16", b"\x1a",
        b"\x00", b"\x1b[C", b"\x1b[D", b"\x1b[B", b"\x1b[A", b"\x18", b"\x03", b"\x05", b"\x1b[6~",
        b"\x1b[5~", b"\x0a", b"xyz", b"\x1b[4~", b"\x1b[1~", b"\x0e", b"\x10", b"\x07", b"\x06"]

def run(binary, keys, doc, path):
    open(path, "w").write(doc)
    pid, fd = pty.fork()
    if pid == 0: os.execv(binary, [binary, "-w", path])
    def pump(t):
        while True:
            r, _, _ = select.select([fd], [], [], t)
            if not r: return
            try: os.read(fd, 65536)
            except OSError: return
    pump(0.3)
    for k in keys: os.write(fd, k); pump(0.03)
    pump(0.2); os.write(fd, b"\x1b"); pump(0.3)                 # ESC saves (-w) and quits
    for _ in range(60):
        if os.waitpid(pid, os.WNOHANG)[0]: break
        time.sleep(0.05)
    else:
        os.kill(pid, 9); os.waitpid(pid, 0); return "HUNG"
    return open(path, "rb").read()

def main():
    a = sys.argv[1:]
    seed = int(a[0]) if len(a) > 0 else 7
    n    = int(a[1]) if len(a) > 1 else 16
    t = tempfile.mkdtemp(prefix="e_fuzz_")
    old = a[2] if len(a) > 2 else None
    if not old:
        old = os.path.join(t, "e.head")
        open(old, "wb").write(subprocess.check_output(["git", "-C", D, "show", "HEAD:e"]))
        os.chmod(old, 0o755)
    new = a[3] if len(a) > 3 else os.path.join(D, "e")
    random.seed(seed); bad = 0
    for i in range(n):
        doc = "".join(f"l{j} some text {j}\n" for j in range(1, random.randint(2, 25)))
        keys = [random.choice(KEYS) for _ in range(random.randint(5, 40))]
        x = run(old, keys, doc, t + "/f.txt"); y = run(new, keys, doc, t + "/f.txt")
        if x != y:
            bad += 1
            print(f"DIFF sequence {i}: {keys}\n  old={x!r}\n  new={y!r}")
    print(f"fuzz: {n - bad}/{n} identical (seed {seed})")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
