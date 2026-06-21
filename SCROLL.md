# Scrolling

e has no draggable scrollbar. It had one; it was buggy, cost a full-buffer
walk per render, and a thumb you drag is the wrong tool when a page jump is
instant (~50µs). Removed.

Pagination lives in the top bar and is always visible:

  [^] [v]   page up / page down (full screen, instant) — click, or PgUp/PgDn
   42%      position readout: Top / Bot / All / NN% through the buffer

Also: mouse wheel (3 lines), type-ahead search. Page-down stops on the last
screenful — it never scrolls into blank past EOF.

The position readout is the one good thing the old scrollbar did, kept as
text: O(1) per keystroke (cached; only recomputed when you scroll or add a
line), no thumb to drag.

A tmux scrollbar is complementary, not a replacement: it scrolls terminal
*scrollback* (lines that left the screen), while e's controls page the
*document*. Run e in tmux if you want both — the controls above stay.
