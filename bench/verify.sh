#!/bin/bash
# The gate a change to e.c has to pass, cheapest first:
#   -Weverything syntax gate -> build -> both pty suites -> screen parity -> edit fuzz -> latency
# Parity and fuzz compare against the COMMITTED binary (git show HEAD:e), so they measure exactly
# what this working tree changed.  Every step runs even if an earlier one fails, so you see the
# whole picture; the exit status is non-zero if anything failed.
#
# A DIFF is only a failure when the change was meant to preserve behaviour: an intentional fix
# (e.g. ^X used to swallow the next key instead of cutting) shows up here by design.
set -o pipefail
D=$(cd "$(dirname "$0")/.." && pwd); cd "$D"
eval "$(sed -n '8,10p' e.c)"                   # W= (warnings) and H= (hardening) from the build header
CC=$(compgen -c clang- 2>/dev/null | grep -xE 'clang-[0-9]+' | sort -t- -k2 -rn | head -1) || CC=clang
fail=0
$CC $W $H -fsyntax-only e.c && echo "gate:    clean ($CC)" || { echo "gate:    FAILED"; fail=1; }
sh e.c || fail=1
python3 bench/core_test.py    | tail -1 || fail=1
python3 bench/pathbar_test.py | tail -1 || fail=1
python3 bench/parity.py       | tail -1 || fail=1
python3 bench/fuzz.py         | tail -1 || fail=1
python3 bench/lat.py e.c arrow
[ $fail = 0 ] && echo "verify:  OK" || echo "verify:  something failed (an intentional behaviour change shows as a fuzz DIFF)"
exit $fail
