#!/bin/bash
# startup benchmark — requires hyperfine
set -e
DIR="$(cd "$(dirname "$0")/.." && pwd)"
TMP=/tmp/e_bench_$$
trap 'rm -f $TMP $TMP.c' EXIT

CC=$(compgen -c clang- 2>/dev/null|grep -xE 'clang-[0-9]+'|sort -t- -k2 -rn|head -1)||CC=""
[[ -z "$CC" ]]&&for c in clang gcc;do command -v $c &>/dev/null&&CC=$c&&break;done
[[ -z "$CC" ]]&&echo "no C compiler"&&exit 1
command -v hyperfine >/dev/null||{ echo "install hyperfine"; exit 1; }

sed 's/lastflag = 0;/update(); _exit(0);/' "$DIR/e.c" > $TMP.c
F="-w -std=gnu89 -O3 -march=native -flto"
$CC $F -static -o $TMP $TMP.c 2>/dev/null||$CC $F -o $TMP $TMP.c

args=(--warmup 3 --min-runs 10 -N -i)
args+=(-n "e"              "$TMP")
args+=(-n "e (5.6k file)"  "$TMP $DIR/e.c")
args+=(-n "ls"             "ls $DIR")
# vi/nano need a pty to exit — use script(1) to provide one
command -v nano >/dev/null && args+=(-n "nano" "script -q /dev/null nano -c ''")
command -v vi   >/dev/null && args+=(-n "vi"   "script -q /dev/null vi -c q")
command -v nvim >/dev/null && args+=(-n "nvim"        "nvim --headless -c q")
command -v nvim >/dev/null && args+=(-n "nvim (file)" "nvim --headless -c q $DIR/e.c")
command -v emacs >/dev/null && args+=(-n "emacs" "emacs -nw -Q --eval '(kill-emacs)'")

hyperfine "${args[@]}"
