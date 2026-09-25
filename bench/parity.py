#!/usr/bin/env python3
# Screen parity between two e binaries: render the same sessions in a pty and compare EVERY cell
# (character + colour + inverse video), not just the text. This is what proves a crunch changed
# no pixels. The top-bar timer field is masked: it is a live latency reading, different every run.
#
#   bench/parity.py                 # committed binary (git show HEAD:e) vs ./e
#   bench/parity.py OLD NEW         # any two binaries
# exits non-zero on any difference, so it can gate a commit.
import os, sys, pty, select, struct, fcntl, termios, subprocess, tempfile, pyte

D = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
COLS, ROWS = 80, 24
TIMER = (COLS - 53, COLS - 44)                     # top-bar ms reading: masked

def screen(binary, args, keys=b""):
    pid, fd = pty.fork()
    if pid == 0:
        fcntl.ioctl(0, termios.TIOCSWINSZ, struct.pack("HHHH", ROWS, COLS, 0, 0))
        os.execv(binary, [binary] + args)
    sc = pyte.Screen(COLS, ROWS); st = pyte.ByteStream(sc)
    def pump(t):
        while True:
            r, _, _ = select.select([fd], [], [], t)
            if not r: return
            try: st.feed(os.read(fd, 65536))
            except OSError: return
    pump(0.5)
    if keys: os.write(fd, keys); pump(0.4)
    cell = lambda y, x: ("*", 0, 0) if (y == 0 and TIMER[0] <= x < TIMER[1]) else \
                        (sc.buffer[y][x].data, sc.buffer[y][x].fg, sc.buffer[y][x].reverse)
    rows = [tuple(cell(y, x) for x in range(COLS)) for y in range(ROWS)]
    os.write(fd, b"\x1b"); pump(0.3)      # quit AFTER the snapshot: e's exit clear (ttclose 2J) blanked every capture, so parity compared empty screens and passed anything
    try: os.waitpid(pid, 0)
    except Exception: pass
    return rows

def main():
    t = tempfile.mkdtemp(prefix="e_parity_")
    old, new = (sys.argv + [None, None])[1:3]
    if not old:
        old = os.path.join(t, "e.head")
        open(old, "wb").write(subprocess.check_output(["git", "-C", D, "show", "HEAD:e"]))
        os.chmod(old, 0o755)
    new = new or os.path.join(D, "e")
    open(t + "/crlf.txt", "wb").write(b"alpha\r\nbeta\r\n\r\ngamma\r\n")
    open(t + "/nonl.txt", "wb").write(b"x\ty tab\nlast no newline")
    open(t + "/empty.txt", "wb").write(b"")
    open(t + "/box.txt", "w").write("box body line\n")
    E = lambda n: os.path.join(D, n)
    cases = [
        ("crlf",            [t + "/crlf.txt"],                  b""),
        ("no trailing nl",  [t + "/nonl.txt"],                  b""),
        ("empty file",      [t + "/empty.txt"],                 b""),
        ("missing file",    [t + "/nope.txt"],                  b""),
        ("small file",      [E("demo.py")],                     b""),
        ("own source",      [E("e.c")],                         b""),
        ("page down x2",    [E("e.c")],                         b"\x1b[6~\x1b[6~"),
        ("select 2 lines",  [E("e.c")],                         b"\x00\x1b[B\x1b[B"),
        ("i-search",        [E("e.c")],                         b"\x06int\x1b"),
        ("edit + undo",     [E("e.c")],                         b"abc\x7f\x1a"),
        ("kill + yank",     [E("e.c")],                         b"\x0b\x0b\x16"),
        ("goto line",       [E("e.c")],                         b"\x07500\r"),
        ("mouse click",     [E("e.c")],                         b"\x1b[<0;10;5M\x1b[<0;10;5m"),
        ("dir browser",     [D],                                b""),
        ("dir filter",      [E("bench")],                       b"co"),
        ("dir path bar",    [E("bench")],                       b"\x0c"),
        ("dir tab",         [E("bench")],                       b"\t"),
        ("--box",           ["--box", "Reply", t + "/box.txt"], b""),
        ("-r reader",       ["-r", E("e.c")],                   b"\x1b[6~\x1b[6~"),
        ("--tail",          ["--tail", E("e.c")],               b""),
        ("+offset",         ["+2000", E("e.c")],                b""),
        ("--nofold",        ["--nofold", E("README")],          b""),
    ]
    bad = 0
    for name, args, keys in cases:
        a, b = screen(old, args, keys), screen(new, args, keys)
        if a == b:
            print(f"SAME  {name}")
            continue
        bad += 1; print(f"DIFF  {name}")
        for y in range(ROWS):
            if a[y] != b[y]:
                x = next(i for i in range(COLS) if a[y][i] != b[y][i])
                txt = lambda r: "".join(c[0] for c in r).rstrip()[:70]
                print(f"   row {y} col {x}: old={a[y][x]} new={b[y][x]}\n   old: {txt(a[y])!r}\n   new: {txt(b[y])!r}")
                break
    print(f"\nparity: {'OK' if not bad else f'{bad} DIFF'}  ({len(cases)} screens, all cells)")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
