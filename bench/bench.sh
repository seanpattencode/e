#!/bin/bash
# startup benchmark — requires hyperfine
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
TMP=/tmp/e_bench_$$
trap 'rm -f $TMP $TMP.c $TMP.ed7 $TMP.ed7.c $TMP.q $TMP.a.c' EXIT

CC=$(compgen -c clang- 2>/dev/null|grep -xE 'clang-[0-9]+'|sort -t- -k2 -rn|head -1)||CC=""
[[ -z "$CC" ]]&&for c in clang gcc;do command -v $c &>/dev/null&&CC=$c&&break;done
[[ -z "$CC" ]]&&echo "no C compiler"&&exit 1
command -v hyperfine >/dev/null||{ echo "install hyperfine"; exit 1; }

sed '/^main(/,/^loop:/s/^loop:/update();_exit(0);\n&/' "$DIR/e.c" > $TMP.c
F="-w -std=gnu89 -O3 -march=native -flto"
{ command -v musl-gcc >/dev/null&&musl-gcc -std=gnu11 -D_GNU_SOURCE -O3 -march=native -flto -w -static -o $TMP $TMP.c 2>/dev/null;}||$CC $F -static -o $TMP $TMP.c 2>/dev/null||$CC $F -o $TMP $TMP.c   # same recipe as sh e.c: musl static first

args=(--warmup 3 --min-runs 10 -N -i)
args+=(-n "e"              "$TMP")
args+=(-n "e (own source)"  "$TMP $DIR/e.c")
args+=(-n "ls"             "ls $DIR")
# vi/nano need a pty to exit — use script(1) to provide one
command -v nano >/dev/null && args+=(-n "nano" "script -q /dev/null nano -c ''")
command -v vi   >/dev/null && args+=(-n "vi"   "script -q /dev/null vi -c q")
command -v nvim >/dev/null && args+=(-n "nvim"        "nvim --headless -c q")
command -v nvim >/dev/null && args+=(-n "nvim (file)" "nvim --headless -c q $DIR/e.c")
command -v emacs >/dev/null && args+=(-n "emacs" "emacs -nw -Q --eval '(kill-emacs)'")

hyperfine "${args[@]}"

# Thompson's ed as it left Bell Labs (V7, 1979), built the same static way: sgtty -> termios in getkey(), mktemp() into a writable 6-X buffer (string literals were writable in 1979), and dropping the free() before realloc() (V7 malloc(3) let you realloc a just-freed block; today that is a double free), is the whole port.
# Source via `cd ~/inspiration && ./pull.sh`. Separate hyperfine run: --input is global, and "q\n" on stdin would hang e.
E7=$HOME/inspiration/src/unix-ed-v7-1979/ed.c
[ -f "$E7" ] && sed 's/<sgtty.h>/<termios.h>/;s/struct sgttyb/struct termios/;s/gtty(0, &b)/tcgetattr(0, \&b)/;s/stty(0, &b)/tcsetattr(0, TCSANOW, \&b)/;s/sg_flags/c_lflag/;/#include <setjmp.h>/a char tfbuf[] = "\/tmp\/eXXXXXX";
s|mktemp("/tmp/eXXXXX")|mktemp(tfbuf)|;/free((char \*)zero);/d' "$E7" > $TMP.ed7.c \
 && { musl-gcc -std=gnu89 -fpermissive -O3 -march=native -flto -w -static -o $TMP.ed7 $TMP.ed7.c 2>/dev/null || $CC -std=gnu89 -w -Wno-error=incompatible-function-pointer-types -Wno-error=return-mismatch -O3 -march=native -flto -static -o $TMP.ed7 $TMP.ed7.c; } \
 && { printf 'q\n' > $TMP.q; LC_ALL=C tr -cd '\0-\177' < "$DIR/e.c" > $TMP.a.c   # V7 ed rejects 8-bit bytes (bit 8 was parity/crypt in 1979): same 1,284 lines, UTF-8 arrows/dashes dropped
   hyperfine --warmup 3 --min-runs 10 -N --input $TMP.q -n "ed V7 1979 (e.c, ascii)" "$TMP.ed7 $TMP.a.c" -n "ed V7 1979" "$TMP.ed7" -n "ed GNU (e.c, ascii)" "ed $TMP.a.c"; }
