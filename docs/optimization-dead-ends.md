# Optimization dead ends

## Static SYMBOL pool (keymapinit)

Replaced 75 individual `malloc(sizeof(SYMBOL))` calls with a static
`SYMBOL sym_pool[128]` and bump allocation. Hypothesis: eliminating
malloc overhead during init would reduce startup time.

Result: no measurable change. Before 556µs, after 562µs (within noise).

Why: glibc malloc serves small allocations from a pre-mapped arena —
no syscall per allocation. At 0.55ms total startup, time is dominated
by kernel exec, dynamic linker, and terminal ioctls, not userspace
allocation.

## Static WINDOW (edinit)

Made the initial WINDOW a static global instead of malloc'd.
One malloc eliminated.

Result: invisible. One malloc is ~50ns. Unmeasurable against 550µs.

## Navigation paths

forwline, backline, forwpage, backpage, update — no malloc in any of
these paths. Nothing to optimize via allocation changes. The screen
update loop is already direct memory writes to VIDEO structs.

## PGO (Profile-Guided Optimization)

Tested clang PGO (`-fprofile-instr-generate` / `-fprofile-instr-use`) with
`-O3 -march=native -flto`. Training workload: open e.c (5,584 lines), render,
exit. Benchmarked startup and 5 common actions at 10,000 iterations each.

Install line tested:

```sh
install) P=/tmp/e_pgo_$$;PD=${CC#clang};command -v llvm-profdata$PD &>/dev/null&&LP=llvm-profdata$PD||LP=llvm-profdata;mkdir -p "$P"&&sed 's/lastflag = 0;/update(); _exit(0);/' "$D/e.c">"$P/t.c"&&$CC -std=gnu89 -O3 -march=native -flto -fprofile-instr-generate -w -o "$P/t" "$P/t.c"&&LLVM_PROFILE_FILE="$P/e.profraw" "$P/t" "$D/e.c"&&$LP merge -output="$P/e.profdata" "$P/e.profraw"&&$CC -std=gnu89 -O3 -march=native -flto -fprofile-instr-use="$P/e.profdata" -w -o "$D/e" "$D/e.c"&&rm -rf "$P"&&mkdir -p "$HOME/.local/bin"&&ln -sf "$D/e" "$HOME/.local/bin/e"&&echo "pgo installed";;
```

Results (hyperfine, clang-23, Linux 6.17):

| Action (10k iter) | Normal (ms) | PGO (ms) | Diff |
|-------------------|-------------|----------|------|
| startup           | 0.47        | 0.51     | PGO 10% slower |
| scroll (pgdn)    | 1.1         | 1.1      | noise |
| cursor move       | 1.1         | 1.1      | noise |
| insert+delete     | 1.3         | 1.3      | noise |
| search ("int")    | 8.1         | 8.1      | noise |
| full redraw       | 127.6       | 128.8    | noise |

Result: no benefit anywhere. Startup regressed ~10%, likely from PGO
reshuffling code layout against this binary's icache profile. The codebase
is too small and the hot paths too short for profile-guided branch prediction
to improve anything. `-O3 -flto` already captures all available optimization.

Reverted to simple build for install.

## What actually dominates startup (0.55ms)

- Kernel process creation: ~100-200µs
- Dynamic linker (libc.so): ~100-200µs
- Terminal init (tcgetattr, ioctl TIOCGWINSZ): ~50µs
- filldir (opendir/readdir/qsort): workload-dependent, ~100µs for small dirs
- Everything else (keymapinit, edinit, update): ~50µs combined

Static linking (`-static`) could cut ~100-200µs but doesn't work on
macOS (Apple removed static libc) and is unreliable on Termux.

## asm, -Os/-O2, and where the time really was (2026-08-29)

Floors on this box (hyperfine -N): bare `_exit` asm binary 120µs, musl
static empty main() 134µs, easm.s (whole viewer in asm) 140µs. e's own
work for a 2-line file was 80µs (init 32, read 21, paint 8, plus an 18µs
empty pre-paint that has since been cut) — so rewriting e in asm could
save at most ~90µs of a ~230µs start; the rest is the kernel's exec.
-O2 and -Os: within noise of -O3.

What was actually slow: (1) inotify — the watch itself is 20µs, but any
process that called inotify_add_watch pays ~1ms±1.5ms of kernel teardown
at exit (synchronize_srcu); a 100ms stat() poll costs 0.6µs. (2) The file
loader — getc per byte + 2x-sized per-line mallocs was 660µs of the 780µs
a 5k-line file took; one read() + exact-size lines cut it by ~150µs. The
per-line malloc/page-fault cost (~400µs/5k lines with musl mallocng) is
the remaining term; an arena per buffer would halve it but needs lfree to
know arena lines.

## Self-timing without perturbing the thing timed (2026-08-30)

The top bar now reports every operation, not just startup. The measurement is
inherently self-referential — you cannot display the cost of the write that
displays it — so the reading is taken at the end of update(), after every row
is composed and queued, and patched into the top-bar cells of that same output
buffer. One write() per frame, unchanged: the repo's own bench/core shows
keystroke p50 12-15µs before and 8-14µs after, i.e. no measurable cost.

The alternative (flush, measure, then a second small write with the true total)
was rejected: it doubles the syscalls per keystroke to add ~5µs of write() to
the number. What is excluded is exactly that final write; everything else —
input decode, command, layout, render, escape generation — is inside.

Format: ms with as many decimals as the 8-char field holds (100ns steps below
10ms, 4 digits: 0.0059ms). clock_gettime(CLOCK_MONOTONIC) itself costs ~20ns
via the vDSO, so 100ns is the honest floor.

## Three crunches that the timer proved too expensive (2026-08-31)

Measured with e's own top-bar reading, so the cost is the number the editor
advertises. All three were reverted.

| cut | tokens saved | what it cost |
|-----|--------------|--------------|
| drop the position cache in update() | 48 | arrow key on a 400k-line file 0.0085ms -> 2.48ms (292x, over the 1ms Sean feels) |
| drop the single-line WFEDIT fast path | 85 | typing 0.0146ms -> 0.1331ms (9x); arrows unaffected |
| exact-sized lines (delete l_size) | 166 | paste 40k chars into one line 51ms -> 366ms (quadratic), typing 8.5x |

The third is the interesting one: with no spare capacity every insert allocates
a fresh LINE, which bumps lgen — and lgen is what invalidates the position
cache. So removing l_size silently disabled the cache from the first cut as
well. Two "independent" optimizations were one.

What stayed: the top-bar button columns are now one table (bar[]) read by both
the painter and the mouse hit-test, so they cannot drift apart; the command
signature is (int k) since the ^U count prefix went; b_bname was always the
basename of b_fname and is now derived where it is shown; the picker's separate
getcwd existed only because b_fname was 80 bytes (it is 1024 since the overflow
fix). 8 -Wno- suppressions were dropped: the crunched code no longer trips
shadow, fallthrough, missing-braces, unused-macros, c++-keyword,
conditional-uninitialized, missing-variable-declarations or documentation.

## What the differential tools caught (bench/parity.py, bench/fuzz.py, bench/lat.py)

Screen parity (every cell, 22 sessions) and the pty suites both passed the whole
crunch. The edit fuzz did not: it found that the i-search rewrite cleared `pat`
on entry, so `^F ^F` no longer repeated the last search and a search dismissed
with ESC left nothing for search-again (Home). Eight hand-written i-search cases
had missed it; a random sequence found it in twelve tries. The pattern is live
again until the first key is typed, and core_test 18 pins it.

Note when reading a fuzz DIFF: it compares against the committed binary, so an
intentional change shows up as a difference. Right now that is `^X` alone (it
used to be a prefix that swallowed the next key). Excluding it, 36/36 random
sequences are byte-identical.
