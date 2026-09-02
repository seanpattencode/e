#!/usr/bin/env python3
# Core editor regression suite (pty + pyte): the paths a whole-file crunch can break —
# file I/O, insert/delete, kill+yank, undo, search, goto, marks, wrap, split, mouse, flags.
# Screen assertions go through pyte so they test the RENDER, not just the byte stream.
import os, pty, select, shutil, struct, subprocess, sys, termios, fcntl, time, re, pyte

E = "/home/seanpatten/e/e"
T = "/tmp/claude-1000/-home-seanpatten-e/325b60d4-6817-438e-bd3b-0a47080c7acb/scratchpad/core_t"
shutil.rmtree(T, ignore_errors=True); os.makedirs(T)
DOC = "\n".join(f"line{i} alpha{i}" for i in range(1, 21)) + "\n"   # line1..line20

passed = 0
def ok(n):
    global passed; passed += 1; print(f"PASS {passed}: {n}")

def wfile(name, body=DOC):
    p = os.path.join(T, name); open(p, "w").write(body); return p

class Ed:
    def __init__(self, args, rows=24, cols=80, cwd=T):
        self.rows, self.cols = rows, cols
        self.m, s = pty.openpty()
        fcntl.ioctl(s, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, 0, 0))
        self.p = subprocess.Popen([E] + args, stdin=s, stdout=s, stderr=s, cwd=cwd,
                                  env=dict(os.environ, TERM="xterm", HOME=T))
        os.close(s); self.buf = b""; self.settle()
    def _read(self, t=0.05):
        r, _, _ = select.select([self.m], [], [], t)
        if r:
            try: self.buf += os.read(self.m, 65536)
            except OSError: pass
    def settle(self, t=0.45):
        end = time.time() + t
        while time.time() < end: self._read()
    def send(self, s, t=0.25):
        os.write(self.m, s if isinstance(s, bytes) else s.encode()); self.settle(t)
    def screen(self):
        sc = pyte.Screen(self.cols, self.rows); st = pyte.Stream(sc)
        st.feed(self.buf.decode("latin1"))
        return [l.rstrip() for l in sc.display]
    def text(self): return "\n".join(self.screen())
    def row(self, i): return self.screen()[i]
    def modeline(self): return self.screen()[self.rows - 2]
    def expect(self, sub, where=None, msg=""):
        hay = self.text() if where is None else self.row(where)
        if sub not in hay:
            sys.stdout.write(f"FAIL {msg}: {sub!r} not on screen\n--- screen ---\n{self.text()}\n")
            self.kill(); sys.exit(1)
    def exited(self, timeout=3.0):
        end = time.time() + timeout
        while time.time() < end:
            rc = self.p.poll()
            if rc is not None: return rc
            self._read()
        self.kill(); sys.exit("FAIL: did not exit")
    def kill(self):
        try: self.p.kill()
        except Exception: pass

# 1. open a file: content renders, modeline names it
f = wfile("a.txt")
e = Ed([f])
e.expect("line1 alpha1", msg="open"); e.expect("line20 alpha20", msg="open")
assert "a.txt" in e.modeline(), f"modeline lacks filename: {e.modeline()!r}"
ok("open file: content renders + modeline shows name")

# 2. insert text
e.send("ZQX"); e.expect("ZQXline1 alpha1", msg="insert")
ok("self-insert types into the buffer")

# 3. backspace deletes
e.send(b"\x7f\x7f"); e.expect("Zline1 alpha1", msg="backspace")
ok("backspace deletes chars")

# 4. C-k kill-line then C-v yank restores it
e.send(b"\x0b")                                   # C-k kill line content
assert "Zline1 alpha1" not in e.text(), "kill-line did not remove the line text"
e.send(b"\x16")                                   # C-v yank
e.expect("Zline1 alpha1", msg="yank")
ok("C-k kill-line + C-v yank restores text")

# 5. C-z undo
before = e.text()
e.send("WWW"); e.expect("WWW", msg="pre-undo")
e.send(b"\x1a")                                   # C-z undo
assert "WWW" not in e.text(), "undo did not remove inserted text"
ok("C-z undo reverts an insert")

# 6. C-g goto-line jumps
e.send(b"\x07"); e.settle(); e.send("12\r")
assert "line12" in e.text(), "goto-line 12: target line not visible"
ok("C-g goto-line reaches the line")

# 7. C-f incremental search finds a match
e.send(b"\x06"); e.settle(); e.send("alpha17")
e.expect("line17 alpha17", msg="isearch")
e.send(b"\x1b")                                   # ESC ends search
ok("C-f i-search locates text")

# 8. C-e goto-eol / C-n,C-p line moves keep rendering sane
e.send(b"\x0e\x0e\x10"); e.expect("line", msg="line moves")
ok("C-n/C-p/C-e line movement renders")

# 9a. C-s save writes to disk (note: e's inotify watcher then re-reads and follows to EOF)
e.send(b"\x13")
assert "line20 alpha20" in open(f).read(), "save lost file content"
assert open(f).read().count("\n") == 20, "save wrote wrong line count"
ok("C-s save writes the buffer to disk")

# 9b. inotify auto-reload: an external change is picked up without a keypress
open(f, "a").write("EXTERNALLY_APPENDED\n"); e.settle(0.6)
e.expect("EXTERNALLY_APPENDED", msg="inotify reload")
ok("inotify auto-reload follows external file changes")

# 10. quit (C-q) exits cleanly
e.send(b"\x11", t=0.1)
assert e.exited() == 0, "C-q did not exit 0"
ok("C-q quits with status 0")

# 10. long line WRAPS across rows (wrap_rows/wrap_render)
long = "W" * 300
g = wfile("wrap.txt", long + "\nshort\n")
e = Ed([g], cols=40)
scr = e.screen()
wrows = [r for r in scr if r.startswith("W")]
assert len(wrows) >= 5, f"long line did not wrap into rows: {len(wrows)}"
assert "".join(wrows).count("W") >= 290, "wrapped rows lost characters"
e.expect("short", msg="post-wrap line")
ok("long line wraps across rows without losing text")
e.send(b"\x11", t=0.1); e.exited()

# 11. -r read-only: buffer not modified by typing
h = wfile("ro.txt")
e = Ed(["-r", h])
e.send("QQQ")
assert open(h).read() == DOC, "read-only mode changed the file"
ok("-r read-only: typing does not alter the file")
e.send(b"\x11", t=0.1); e.exited()

# 12. +offset starts at a byte offset
off = DOC.index("line15")
e = Ed([f"+{off}", wfile("off.txt")])
e.expect("line15", msg="+offset")
ok("+offset positions into the file")
e.send(b"\x11", t=0.1); e.exited()

# 13. --tail opens at the end
e = Ed(["--tail", wfile("tail.txt")])
e.expect("line20 alpha20", msg="--tail")
ok("--tail opens at end of file")
e.send(b"\x11", t=0.1); e.exited()

# 14. query-replace (C-h): prompts, then replaces through the buffer
q = wfile("qr.txt")
e = Ed([q])
e.send(b"\x08"); e.settle()                        # C-h query-replace
e.expect("Old string", msg="query-replace prompt")  # readpattern("Old string")
e.send("alpha3\r"); e.settle(); e.send("BETA\r"); e.settle()
e.send("!")                                        # ! = replace all
e.expect("BETA", msg="query-replace applied")
ok("C-h query-replace prompts and replaces")
e.send(b"\x11", t=0.1); e.exited()

# 15. SGR mouse click positions the cursor (row 4 -> line3)
e = Ed([wfile("ms.txt")])
e.send(b"\x1b[<0;3;4M\x1b[<0;3;4m")               # press+release col3 row4
e.send(b"\x0b")                                   # C-k kills the clicked line
assert "line3 alpha3" not in e.text(), "mouse click did not land on line3"
ok("SGR mouse click positions the cursor")
e.send(b"\x11", t=0.1); e.exited()

# 16. dir browser opens on a directory argument
e = Ed([T])
e.expect("a.txt", msg="dir browser")
assert T.split("/")[-1] in e.text(), "dir view missing current path"
ok("directory argument opens the browser")
e.send(b"\x1b", t=0.1); e.exited()

# 17. top-bar timer: every operation reports its own key->screen time, at 100ns resolution
e = Ed([wfile("t.txt")], cols=132)
TCOL = 132 - 53                                   # the timer field: 8 chars, then a space, then the position readout
def timer(): return e.row(0)[TCOL:TCOL + 8].strip()
def ms(t):
    assert re.fullmatch(r"\d+(\.\d+)?ms", t), f"malformed timer reading {t!r}"
    return float(t[:-2])
boot = ms(timer())                                # startup: main() -> first frame
assert 0 < boot < 500, f"implausible startup reading {boot}ms"
seen = set()
for k in (b"\x1b[B", b"\x1b[B", b"x", b"\x7f", b"\x1b[6~", b"\x06"):
    e.send(k); v = ms(timer())
    assert 0 < v < 500, f"implausible reading {v}ms after {k!r}"
    seen.add(v)
assert len(seen) > 1, f"timer never changed across operations: {seen}"
assert min(seen) < boot, "no operation beat startup — timer looks frozen at the boot value"
assert timer() != "0.0000ms", "timer reads zero"
ok("top-bar timer reports each operation, not just startup")
e.send(b"\x1b", t=0.1)                            # first ESC ends the i-search the loop left open
e.send(b"\x1b", t=0.1); e.exited()

# 18. i-search keeps the last pattern: ^F^F repeats it, and a search dismissed without typing
#     still leaves it for search-again (Home). Regression: a rewrite cleared pat on entry.
SDOC = "alpha one\nbeta two\nalpha three\ngamma four\n"
e = Ed([wfile("s.txt", SDOC)])
e.send(b"\x06alpha\x1b")                          # i-search alpha, ESC ends it on the 1st match
e.send(b"\x06\x06\x1b"); e.send("X")             # reopen, ^F repeats -> 2nd match
e.expect("alphaX three", msg="^F^F repeat last search")
e.send(b"\x1b", t=0.1); e.exited()

e = Ed([wfile("s2.txt", SDOC)])
e.send(b"\x06alpha\x1b")
e.send(b"\x06\x1b")                               # opened and dismissed without typing
e.send(b"\x1b[1~"); e.send("X")                    # Home = search-again, pattern must have survived
e.expect("alphaX three", msg="search-again after an untyped i-search")
ok("i-search retains the last pattern for repeat and search-again")
e.send(b"\x1b", t=0.1); e.exited()

# 19. picker: typing searches the whole tree below the folder — this folder's hits rank first,
#     deeper ones are shown by the path they came from, and Enter opens them.
R = os.path.join(T, "rec"); os.makedirs(R + "/src/deep")
open(R + "/report.txt", "w").write("top\n")
open(R + "/src/report_helper.c", "w").write("mid\n")
open(R + "/src/deep/report_deep.md", "w").write("low\n")
e = Ed([R])
e.send("report")
e.expect("find: report (1 here, 2 below)", msg="here/below counts")
hits = [l.strip() for l in e.screen() if "report" in l and "find:" not in l]
assert hits[0] == "report.txt",                 f"this folder's hit must rank first: {hits[:3]}"
assert hits[1] == "src/report_helper.c",        f"one level down comes next: {hits[:3]}"
assert hits[2] == "src/deep/report_deep.md",    f"then deeper: {hits[:3]}"
ok("picker searches below the folder: local first, deeper by depth, shown by path")
e.send(b"\x1b", t=0.1); e.exited()

e = Ed([R])
e.send("report_deep"); e.send(b"\r")
assert "src/deep/report_deep.md" in e.modeline(), f"Enter on a deeper hit must open it: {e.modeline()!r}"
e.expect("low", msg="deeper file contents")
ok("Enter on a deeper hit opens it by its path")
e.send(b"\x1b", t=0.1); e.exited()

print(f"\n{passed}/{passed} PASS")
