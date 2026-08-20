#!/usr/bin/env python3
# pty e2e for e's file picker:
#  - the current path is shown as a WRAPPING header row (full path visible on open, even in a thin window)
#  - ^L = editable address bar (cwd pre-filled), path wraps too so a thin window stays editable
#  - typing filters; Tab cycles entries (skips the header); Enter opens dir/file
import os, pty, select, shutil, struct, subprocess, sys, termios, fcntl, time, re, signal, pyte

E = "/home/seanpatten/e/e"
T = "/tmp/claude-1000/-home-seanpatten-e/325b60d4-6817-438e-bd3b-0a47080c7acb/scratchpad/pathbar_t"
shutil.rmtree(T, ignore_errors=True)
HOME = T + "/home"; DIRA = T + "/dirA"
DEEP = T + "/deep" + "/d123456789" * 16          # 274 chars: past old dirsrch[64]/dgo[160] caps
for d in (HOME, DIRA + "/sub", DEEP):
    os.makedirs(d)
open(DIRA + "/sub/f.txt", "w").write("hello\n")
open(DIRA + "/other.txt", "w").write("x\n")      # unique to DIRA (sub/ has only f.txt) -> dir discriminator
open(DEEP + "/g.txt", "w").write("deep\n")
OUT = T + "/out"
HINT = "go: Enter=open file/dir"                  # bottom line in ^L path mode
LIST = "type to filter"                           # listing-mode bottom line ("N items · type to filter · ^L edit path")

ANSI = re.compile(rb'\x1b\[[0-9;?]*[A-Za-z]|\x1b\][^\x07]*\x07|\x1b[()][A-Za-z0-9]|\x1b[=>]')
def flat(b): return ANSI.sub(b'', b).translate(None, b' \r\n\t')   # wrapped rows rejoin into the contiguous path
def hdr_inverse(e, path):                         # the path renders as the inverse (white) header: an ESC[7m sits just before it
    i = e.buf.find(path.encode())
    assert i > 0 and b"\x1b[7m" in e.buf[max(0,i-16):i], f"header not inverse before {path}"
def no_modeline_path(e):                           # dir mode drops the truncated File: path from the modeline
    assert b"File:" not in e.buf, "modeline still shows a File: path in dir mode"

passed = 0
def ok(name):
    global passed; passed += 1; print(f"PASS {passed}: {name}")

class Ed:
    def __init__(self, args, cwd, rows=40, cols=220):
        self.m, s = pty.openpty()
        fcntl.ioctl(s, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, 0, 0))
        env = dict(os.environ, HOME=HOME, TERM="xterm")
        self.p = subprocess.Popen([E] + args, stdin=s, stdout=s, stderr=s, cwd=cwd, env=env)
        os.close(s); self.buf = b""
    def _read(self, t=0.05):
        r, _, _ = select.select([self.m], [], [], t)
        if r:
            try: self.buf += os.read(self.m, 65536)
            except OSError: pass
    def wait_for(self, needle, timeout=3.0):
        needle = needle.encode() if isinstance(needle, str) else needle
        end = time.time() + timeout
        while time.time() < end:
            if needle in self.buf: return True
            self._read()
        sys.stdout.write(f"FAIL waiting for {needle!r}\n--- last 600 ---\n{self.buf[-600:]!r}\n"); self.kill(); sys.exit(1)
    def shows(self, path, extra="", timeout=3.0):   # poll until `extra` present AND full path reconstructs from (wrapped) screen
        pe = path.encode(); xe = extra.encode(); end = time.time() + timeout
        while time.time() < end:
            if xe in self.buf and pe in flat(self.buf): return True
            self._read()
        sys.stdout.write(f"FAIL path not on screen: {path} (need {extra!r})\n--- flat tail ---\n{flat(self.buf)[-500:]!r}\n"); self.kill(); sys.exit(1)
    def clear(self): self.buf = b""
    def drain(self, t=0.3):                          # keep reading (a render can arrive in a later loop iteration than the HINT)
        end = time.time() + t
        while time.time() < end: self._read()
    def resize(self, rows, cols):                    # set the pty size, then deliver SIGWINCH (child isn't the ctty session leader here; a real foot is)
        fcntl.ioctl(self.m, termios.TIOCSWINSZ, struct.pack("HHHH", rows, cols, 0, 0))
        self.p.send_signal(signal.SIGWINCH); time.sleep(0.1)
    def send(self, s): os.write(self.m, s if isinstance(s, bytes) else s.encode()); time.sleep(0.08)
    def exited(self, timeout=3.0):                   # drain while waiting: a big render fills the 64KB pty buffer and e blocks on write before exit()
        end = time.time() + timeout
        while time.time() < end:
            rc = self.p.poll()
            if rc is not None: return rc
            self._read()
        self.kill(); sys.exit("FAIL: did not exit")
    def kill(self):
        try: self.p.kill()
        except Exception: pass

# --- ON OPEN: the full current path is shown as an inverse header at the top; no cut path in the modeline ---
e = Ed(["--pick", OUT, DIRA], cwd=T)
e.shows(DIRA, LIST); hdr_inverse(e, DIRA); no_modeline_path(e)
ok("on open: full path visible as inverse header (no ^L), no File: in modeline")

# --- Tab cycles real entries, skipping the header (verifies header-offset mapping) ---
e.clear(); e.send("\t"); e.wait_for("(all) -> sub"); ok("Tab selects a real entry (skips header)")

# --- ^L pre-fills cwd as an editable path ---
e.clear(); e.send(b"\x0c"); e.shows(DIRA + "/", HINT); ok("^L: path pre-filled with cwd/")

# --- edit path + Enter enters subdir (f.txt is unique to sub) ---
e.send("sub"); e.send(b"\r"); e.shows(DIRA + "/sub", LIST); e.wait_for("f.txt"); ok("edit path + Enter enters subdir")

# --- ^L, backspace x4 trims 'sub/', Enter -> parent (other.txt is unique to DIRA) ---
e.clear(); e.send(b"\x0c"); e.shows(DIRA + "/sub/", HINT)
e.send(b"\x7f" * 4); e.send(b"\r"); e.wait_for("other.txt"); e.wait_for(LIST); ok("backspace-edit + Enter goes to parent")

# --- plain typing still filters the listing ---
e.clear(); e.send("su"); e.wait_for("find: su (1/3)"); ok("plain typing filters (1/3; '..','sub','other.txt')")
e.send(b"\x7f\x7f")

# --- '/'-first is an absolute go path, not a filter, no cwd seeding ---
ABS = "/zqxabs9931/leafZZZ"
e.clear(); e.send(ABS); e.shows(ABS, HINT); ok("'/'-typed path is absolute (no cwd seed)")
e.send(b"\x7f" * len(ABS))

# --- TYPE a relative path (no ^L, no leading '/'): the '/' turns it into a path bar ---
e.clear(); e.send("sub/"); e.shows("sub/", HINT); ok("typing 'sub/' switches to path mode (no ^L needed)")
e.send("f.txt"); e.send(b"\r")
assert e.exited() == 0 and open(OUT).read() == os.path.realpath(DIRA + "/sub/f.txt"), open(OUT).read()
ok("typed relative path + Enter picks the file")
os.remove(OUT); os.remove(HOME + "/.e_pick")
e = Ed(["--pick", OUT, DIRA], cwd=T); e.shows(DIRA, LIST)

# --- ^L path edited to a FILE picks it ---
e.clear(); e.send(b"\x0c"); e.shows(DIRA + "/", HINT)
e.send("sub/f.txt"); e.shows(DIRA + "/sub/f.txt", HINT); e.send(b"\r")
assert e.exited() == 0 and open(OUT).read() == os.path.realpath(DIRA + "/sub/f.txt"), open(OUT).read()
ok("picker: ^L path to file writes realpath + exits 0")

# --- long path (274 chars) survives header + ^L + pick ---
os.remove(OUT); os.remove(HOME + "/.e_pick")
e = Ed(["--pick", OUT, DEEP], cwd=T)
e.shows(DEEP, LIST); ok("on open: 274-char path fully shown (wrapped header)")
e.clear(); e.send(b"\x0c"); e.shows(DEEP + "/", HINT)
e.send("g.txt"); e.send(b"\r")
assert e.exited() == 0 and open(OUT).read() == os.path.realpath(DEEP + "/g.txt")
ok("long path: ^L pre-fill + pick")

# --- non-pick: e on a dir, ^L to a file opens it in the editor ---
e = Ed([DIRA], cwd=T)
e.shows(DIRA, LIST)
e.clear(); e.send(b"\x0c"); e.shows(DIRA + "/", HINT)
e.send("sub/f.txt"); e.send(b"\r")
e.wait_for("hello"); ok("editor mode: ^L path opens the file")
e.clear(); e.send(b"\x0c"); time.sleep(0.3)
assert HINT.encode() not in e.buf, "refresh leaked a go-bar in file mode"
ok("^L in file mode stays plain refresh")
e.send(b"\x1b"); e.exited()

# ===== THE SCREAM: thin window shows the FULL path (wrapped), never truncated =====
os.remove(HOME + "/.e_pick")
e = Ed(["--pick", OUT, DEEP], cwd=T, rows=24, cols=34)   # 32 usable cols -> path wraps to ~9 rows
e.shows(DEEP, LIST); hdr_inverse(e, DEEP[:20]); no_modeline_path(e)
ok("THIN window ON OPEN: full path wraps into inverse header (no ^L, not truncated)")
e.clear(); e.send(b"\x0c"); e.shows(DEEP + "/", HINT); ok("thin window: ^L path also wraps (editable)")
e.send("g.txt"); e.send(b"\r")
assert e.exited() == 0 and open(OUT).read() == os.path.realpath(DEEP + "/g.txt"), open(OUT).read()
ok("thin window: wrapped path edited + picks")

# --- tiny window: path taller than window -> w_skip keeps the edit point (tail) visible ---
os.remove(OUT); os.remove(HOME + "/.e_pick")
e = Ed(["--pick", OUT, DEEP], cwd=T, rows=8, cols=30)
e.wait_for("type to filter"); e.drain()            # let the on-open render finish (message line draws before the buffer rows) ...
e.clear(); e.send(b"\x0c"); e.wait_for(HINT); e.drain()   # ... so clear() wipes it and scr is the ^L render only
scr = flat(e.buf)
assert (DEEP + "/")[-24:].encode() in scr, "tiny window: edit-point (tail) not visible"
assert DEEP[:20].encode() not in scr, "tiny window: head should have scrolled off (w_skip)"
ok("tiny window: w_skip keeps the edit point (tail) on screen")
e.send(b"\x1b"); e.exited()

# --- RESIZE (SIGWINCH) reflows immediately, no keypress: foot execs e at 80x24 then tiles the window ---
os.remove(HOME + "/.e_pick") if os.path.exists(HOME + "/.e_pick") else None
e = Ed(["--pick", OUT + ".rz", DIRA], cwd=T, rows=24, cols=80)
e.shows(DIRA, LIST); e.drain()
n0 = len(e.buf)
e.resize(40, 45); e.drain(0.5)                     # narrower+taller, like sway tiling the foot picker
assert len(e.buf) > n0, "no reflow emitted on SIGWINCH (resize not processed until a keypress)"
scr = pyte.Screen(45, 40); st = pyte.Stream(scr); st.feed(e.buf.decode("latin1"))
disp = [l.rstrip() for l in scr.display]
assert disp[1].startswith("/") and "pathbar_t/dir" in "".join(disp[1:4]), f"after resize, path header not at top: {disp[:4]}"
ok("resize (SIGWINCH) reflows now: header back at top, no keypress")
e.send(b"\x1b"); e.exited()

# --- >63-char filename: Dent.n[64] truncated it, realpath failed, Enter/click did nothing (WhatsApp/Downloads docs) ---
LONG = "WhatsApp Document 2026-08-20 at 17.44.47 Patten_Sean_quarterly_report_final_v3_signed_copy.pdf"   # 94 chars
open(DIRA + "/" + LONG, "w").write("doc\n")
for f in (OUT, HOME + "/.e_pick"):
    if os.path.exists(f): os.remove(f)
e = Ed(["--pick", OUT, DIRA], cwd=T); e.wait_for("^L edit path")
e.send("Whats"); e.wait_for("(1/4)"); e.send(b"\r")
assert e.exited() == 0 and open(OUT).read() == os.path.realpath(DIRA + "/" + LONG), open(OUT).read()
ok("94-char filename: filter + Enter picks the full path (was a silent no-op at 63)")

# --- ESC quits picker without picking ---
e = Ed(["--pick", OUT + ".none", DIRA], cwd=T)
e.wait_for("^L edit path"); e.send(b"\x1b")
assert e.exited() == 0 and not os.path.exists(OUT + ".none")
ok("ESC quits picker, nothing written")

print(f"\n{passed}/{passed} PASS")
