#if 0
# sh e.c [build|install|debug|clean] — self-compiling editor
[ -z "$BASH_VERSION" ] && exec bash "$0" "$@"
set -e; D="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CC=$(compgen -c clang- 2>/dev/null|grep -xE 'clang-[0-9]+'|sort -t- -k2 -rn|head -1)||CC=""
[[ -z "$CC" ]]&&for c in clang gcc;do command -v $c &>/dev/null&&CC=$c&&break;done
[[ -z "$CC" ]]&&echo "no C compiler"&&exit 1
W="-std=gnu89 -Werror -Weverything -Wno-padded -Wno-disabled-macro-expansion -Wno-reserved-id-macro -Wno-documentation -Wno-declaration-after-statement -Wno-c99-extensions -Wno-gcc-compat -Wno-unsafe-buffer-usage -Wno-used-but-marked-unused -Wno-unused-parameter -Wno-missing-braces -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-cast-qual -Wno-missing-variable-declarations -Wno-implicit-int-conversion -Wno-conditional-uninitialized -Wno-shadow -Wno-unused-macros -Wno-implicit-fallthrough -Wno-unused-function -Wno-unused-result -Wno-implicit-void-ptr-cast -Wno-c++-keyword --system-header-prefix=/usr/include"
H="-fstack-protector-strong -ftrivial-auto-var-init=zero -D_FORTIFY_SOURCE=2"
[[ "$(uname -m)" == x86_64 ]]&&H+=" -fstack-clash-protection -fcf-protection"
case "${1:-build}" in
build)   $CC $W $H -fsyntax-only "$D/e.c"& $CC -std=gnu89 -O3 -march=native -flto -w -o "$D/e" "$D/e.c";;
debug)   $CC $W $H -Wl,-z,relro,-z,now -O1 -g -fsanitize=address,undefined,integer -o "$D/e" "$D/e.c";;
install) sh "$D/e.c"&&mkdir -p "$HOME/.local/bin"&&ln -sf "$D/e" "$HOME/.local/bin/e"&&echo "✓ ~/.local/bin/e";;
clean)   rm -f "$D/e";;
*)       echo "Usage: sh e.c [build|install|debug|clean]";;
esac
exit 0
#endif
#define	KBLOCK	4096
#define	GOOD	0

#define	BDC1	'/'
#define	GOSLING	1

#define	NROW	66
#define	NCOL	132

#define	KUP	K01
#define	KDOWN	K02
#define	KLEFT	K03
#define	KRIGHT	K04
#define	KFIND	K05
#define	KINSERT	K06
#define	KREMOVE	K07
#define	KSELECT	K08
#define	KPREV	K09
#define	KNEXT	K0A
#define	KF4	K0B
#define	KF6	K0C
#define	KF7	K0D
#define	KF8	K0E
#define	KF9	K0F
#define	KF10	K10
#define	KF11	K11
#define	KF12	K12
#define	KF13	K13
#define	KF14	K14
#define	KHELP	K15
#define	KDO	K16
#define	KF17	K17
#define	KF18	K18
#define	KF19	K19
#define	KF20	K1A
#define	KPF1	K1B
#define	KPF2	K1C
#define	KPF3	K1D
#define	KPF4	K1E
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<unistd.h>
#include	<sys/wait.h>
#include	<sys/inotify.h>
#include	<sys/select.h>
#include	<signal.h>
static int dirmode,pmode,nosb;
static unsigned char rbuf[4096];static int rh,rt;
static char dirsrch[64];
static int dirsl;
static int sb_top, sb_bot;
static int uc[2048],ut,ul,hoff;
static char *box_msg;
static int fold_a=1;
/* read-only / bookmark-mode globals (for `a book read` integration) */
static int ro_flag;
static long start_off = -1;
static const char *pos_out_path;
static void write_pos(void);
#define LSA(lp) (llength(lp)>=12&&!memcmp((lp)->l_text,"## a-loaded ",12))
#define LSE(lp) (llength(lp)>=15&&!memcmp((lp)->l_text,"## a-loaded-end",15))
#define FSKIP(lp,bp) do{lp=lforw(lp);while(lp!=(bp)->b_linep&&!LSE(lp))lp=lforw(lp);if(lp!=(bp)->b_linep)lp=lforw(lp);}while(0)

#define	CVMVAS	1

#define	NSHASH	31
#define	NFILEN	80
#define	NBUFN	16
#define	NLINE	256
#define	NKBDM	256
#define NMSG	256
#define	NPAT	80
#define	HUGE	1000
#define NSRCH	128
#define	NXNAME	64

#define	FALSE	0
#define	TRUE	1
#define	ABORT	2

#define	CFCPCN	0x0001
#define	CFKILL	0x0002

#define	FIOSUC	0
#define	FIOFNF	1
#define	FIOEOF	2
#define	FIOERR	3

#define	DIOSUC	0
#define	DIOEOF	1
#define	DIOERR	2

#define	CNONE	0
#define	CTEXT	1
#define	CMODE	2

#define	EFNEW	0x0001
#define	EFAUTO	0x0002
#define	EFCR	0x0004

#define	NKEYS	2048

#define	METACH	0x1B
#define	CTMECH	0x1C
#define	EXITCH	0x1D
#define	CTRLCH	0x1E
#define	HELPCH	0x1F

#define	KCHAR	0x00FF
#define	KCTRL	0x0100
#define	KMETA	0x0200
#define	KCTLX	0x0400

#define	KFIRST	0x0080
#define	KLAST	0x009F

#define	KRANDOM	0x0080
#define	K01	0x0081
#define	K02	0x0082
#define	K03	0x0083
#define	K04	0x0084
#define	K05	0x0085
#define	K06	0x0086
#define	K07	0x0087
#define	K08	0x0088
#define	K09	0x0089
#define	K0A	0x008A
#define	K0B	0x008B
#define	K0C	0x008C
#define	K0D	0x008D
#define	K0E	0x008E
#define	K0F	0x008F
#define	K10	0x0090
#define	K11	0x0091
#define	K12	0x0092
#define	K13	0x0093
#define	K14	0x0094
#define	K15	0x0095
#define	K16	0x0096
#define	K17	0x0097
#define	K18	0x0098
#define	K19	0x0099
#define	K1A	0x009A
#define	K1B	0x009B
#define	K1C	0x009C
#define	K1D	0x009D
#define	K1E	0x009E
#define	K1F	0x009F

#define	_W	0x01
#define	_U	0x02
#define	_L	0x04
#define	_C	0x08

#define	ISWORD(c)	((cinfo[(unsigned char)(c)]&_W)!=0)
#define	ISCTRL(c)	((cinfo[(unsigned char)(c)]&_C)!=0)
#define	ISUPPER(c)	((cinfo[(unsigned char)(c)]&_U)!=0)
#define	ISLOWER(c)	((cinfo[(unsigned char)(c)]&_L)!=0)
#define	TOUPPER(c)	((c)-0x20)
#define	TOLOWER(c)	((c)+0x20)

typedef	struct	SYMBOL {
	struct	SYMBOL *s_symp;
	short	s_nkey;
	char	*s_name;
	int	(*s_funcp)(int, int, int);
}	SYMBOL;

typedef	struct	WINDOW {
	struct	WINDOW *w_wndp;
	struct	BUFFER *w_bufp;
	struct	LINE *w_linep;
	struct	LINE *w_dotp;
	short	w_doto;
	struct	LINE *w_markp;
	short	w_marko;
	char	w_toprow;
	char	w_ntrows;
	char	w_force;
	char	w_flag;
	int	w_skip;	/* wrap rows skipped from top of w_linep (for visual-row scrolling) */
}	WINDOW;

#define	WFFORCE	0x01
#define	WFMOVE	0x02
#define	WFEDIT	0x04
#define	WFHARD	0x08
#define	WFMODE	0x10

typedef	struct	BUFFER {
	struct	BUFFER *b_bufp;
	struct	LINE *b_dotp;
	short	b_doto;
	struct	LINE *b_markp;
	short	b_marko;
	struct	LINE *b_linep;
	char	b_nwnd;
	char	b_flag;
	char	b_fname[NFILEN];
	char	b_bname[NBUFN];
}	BUFFER;

#define	BFCHG	0x01
#define	BFBAK	0x02

typedef	struct	{
	struct	LINE *r_linep;
	short	r_offset;
	int	r_size;
}	REGION;

typedef	struct	LINE {
	struct	LINE *l_fp;
	struct	LINE *l_bp;
	short	l_size;
	short	l_used;
	char	l_text[1];
}	LINE;

#define	lforw(lp)	((lp)->l_fp)
#define	lback(lp)	((lp)->l_bp)
#define	lgetc(lp, n)	((lp)->l_text[(n)]&0xFF)
#define	lputc(lp, n, c)	((lp)->l_text[(n)]=(c))
#define	llength(lp)	((lp)->l_used)

extern	int	thisflag;
extern	int	lastflag;
extern	int	curgoal;
extern	int	epresf;
extern	int	sgarbf;
extern	WINDOW	*curwp;
extern	BUFFER	*curbp;
extern	WINDOW	*wheadp;
extern	BUFFER	*bheadp;
extern	BUFFER	*blistp;
extern	short	kbdm[];
extern	short	*kbdmip;
extern	short	*kbdmop;
extern	char	pat[];
extern	SYMBOL	*symbol[];
extern	SYMBOL	*binding[];

static void	ttflush(void);
static void	eprintf(char*, ...);
static void	ttputc(int);
static void	asciiparm(int);
static void	ttwindow(int, int);
static void	update(void);
static void	eputc(int);
static void	eerase(void);
static int	ctrlg(int, int, int);
static int	quit(int, int, int);
static int	backdir(int, int, int);
static int	filldir(char *);
static int	readin(char *);
static void	keydup(int, char *);
static void	keyadd(int, int (*)(int,int,int), char *);
static int	makelist(void);
static int	popblist(void);
static int	bclear(BUFFER *);
static int	addline(char *);
static void	itoa_(char *, int, int);
static int	forwsrch(void);
static int	backsrch(void);
static int	readpattern(char *);
static int	inword(void);
static int	getregion(REGION *);
static int	setsize(REGION *, long);
static void	setgoal(void);
static int	getgoal(LINE *);
static int	execute(int, int, int);
static int	writeout(char *);
static int	getkey(void);
static int	getctl(void);
static void	keyname(char *, int);
static int	symhash(char *);
static void	modeline(WINDOW *);
static void	vtmove(int, int);
static void	vtputc(int);
static void	vteeol(void);
static int	wrap_rows(LINE *);
static int	eread(char *, char *, int, int, va_list);
static void	eformat(char *, va_list);
static void	eputi(int, int);
static void	eputs(char *);
static int	is_find(int);
static void	is_cpush(int);
static void	is_lpush(void);
static void	is_pop(void);
static int	is_peek(void);
static int	is_undo(int *, int *);
static void	is_prompt(int, int, int);
static void	is_dspl(char *, int);
static int	isearch(int);
static int	getkbd(void);
static int	speak_line(int, int, int);
static int	stop_speak(int, int, int);
static int	speak_running(void);
static int	eyesno(char *);
static int	writemsg(char *);
static int	readmsg(int, int, int);
static int	ffropen(char *);
static int	ffwopen(char *);
static int	ffclose(void);
static int	ffputline(char *, int);
static int	ffgetline(char **, int *);
static int	fbackupfile(char *);
static void	adjustcase(char *);
static void	makename(char *, char *);
static void	edinit(char *);
static void	vtinit(void);
static void	vttidy(void);
static int	anycb(void);
static void	kdelete(void);
static int	kinsert(int);
static int	kremove(int);
static void	setscores(int, int);
static void	traceback(int, int, int, int);
static int	getxtra(SYMBOL *, SYMBOL *, int);
static int	lnewline(void);
static int	ldelnewline(void);
static void	lchange(int);
static int	backchar(int, int, int);
static int	forwchar(int, int, int);
static int	eq(int, int);
static int	undo(int, int, int);
static BUFFER	*bfind(char *, int);
static BUFFER	*bcreate(char *);
static WINDOW	*wpopup(void);
static LINE	*lalloc(int);
static SYMBOL	*symlookup(char *);
extern  int	nrow;
extern  int	ncol;
extern	char	*version[];
extern	int	ttrow;
extern	int	ttcol;
extern	int	tceeol;
extern	int	tcinsl;
extern	int	tcdell;
extern	char	cinfo[];
extern	char	*keystrings[];
extern	int	nmsg;
extern	int	curmsgf;
extern	int	newmsgf;
extern	char	msg[];

char	cinfo[256] = {
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	_C,		_C,		_C,		_C,
	0,		0,		0,		0,
	_W,		0,		0,		_W,
	0,		0,		0,		0,
	0,		0,		0,		0,
	_W,		_W,		_W,		_W,
	_W,		_W,		_W,		_W,
	_W,		_W,		0,		0,
	0,		0,		0,		0,
	0,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		0,
	0,		0,		0,		_W,
	0,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		0,
	0,		0,		0,		_C,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	0,		0,		0,		0,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	0,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		_U|_W,		_U|_W,
	_U|_W,		_U|_W,		0,		_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	0,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		_L|_W,		_L|_W,
	_L|_W,		_L|_W,		0,		0
};
#include	<termios.h>
#include	<sys/ioctl.h>

#define	NOBUF	512

static char	obuf[NOBUF];
static int	nobuf;
static struct	termios	oldtty;
static struct	termios	newtty;
int	nrow;
int	ncol;

static void
ttopen(void)
{
	struct winsize ws;

	tcgetattr(1, &oldtty);
	newtty = oldtty;

#ifdef __sun
	newtty.c_iflag &= ~(IMAXBEL|IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	newtty.c_oflag &= ~OPOST;
	newtty.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	newtty.c_cflag &= ~(CSIZE|PARENB);
	newtty.c_cflag |= CS8;
#else
        cfmakeraw(&newtty);
#endif

	tcsetattr(1, TCSADRAIN, &newtty); tcflush(0, TCIFLUSH);

	if (ioctl(0, TIOCGWINSZ, &ws) == 0 && ws.ws_row && ws.ws_col) {
		nrow = ws.ws_row;
		ncol = ws.ws_col;
	} else {
		nrow = 24;
		ncol = 80;
	}

	if (nrow > NROW)
		nrow = NROW;
	if (ncol > NCOL)
		ncol = NCOL;

	write(1, "\033[?1006h\033[?1002h\033[?2004h", 24);
}

static void
ttclose(void)
{
	write(1,"\033[?1002l\033[?1006l\033[?2004l\033[2J\033[H",31); ttflush();
	tcflush(0, TCIFLUSH); tcsetattr(1, TCSADRAIN, &oldtty);
}

static void
ttputc(int c)
{
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
}

static void
ttflush(void)
{
	if (nobuf != 0) {
		write(1, obuf, nobuf);
		nobuf = 0;
	}
}

static int ifd=-1,iwd=-1;
static void fwatch(const char*f){char d[NFILEN];const char*s=strrchr(f,'/');
    if(s){size_t n=(size_t)(s-f);memcpy(d,f,n);d[n]=0;}else strcpy(d,".");
    if(ifd<0)ifd=inotify_init1(IN_NONBLOCK|IN_CLOEXEC);
    if(iwd>=0)inotify_rm_watch(ifd,iwd);
    iwd=ifd<0?-1:inotify_add_watch(ifd,d,IN_CLOSE_WRITE|IN_MOVED_TO|IN_MODIFY);
}
static int
ttgetc(void)
{
	if(rh<rt)return rbuf[rh++];
	for(;;){fd_set r;FD_ZERO(&r);FD_SET(0,&r);int mx=0;
	    if(ifd>=0){FD_SET(ifd,&r);mx=ifd;}
	    if(select(mx+1,&r,0,0,0)<0)continue;
	    if(ifd>=0&&FD_ISSET(ifd,&r)){char ib[4096];ssize_t n=read(ifd,ib,sizeof ib);char*p=ib;int hit=0;
	        const char*b=strrchr(curbp->b_fname,'/');b=b?b+1:curbp->b_fname;
	        while(n>0&&p<ib+n){struct inotify_event*e=(void*)p;
	            if(e->len&&!strcmp(e->name,b))hit=1;p+=sizeof(*e)+e->len;}
	        if(hit&&!(curbp->b_flag&BFCHG)){LINE*lp;readin(curbp->b_fname);
	            for(lp=lforw(curbp->b_linep);lforw(lp)!=curbp->b_linep;lp=lforw(lp));
	            curwp->w_dotp=lp;curwp->w_doto=llength(lp);
	            curwp->w_flag|=WFHARD;update();ttflush();}}
	    if(FD_ISSET(0,&r)){int n=read(0,rbuf,sizeof rbuf);if(n>0){rh=1;rt=n;return rbuf[0];}}}
}

#define	BEL	0x07
#define	ESC	0x1B
#define	LF	0x0A

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

int	tceeol	=	3;
int	tcinsl	= 	17;
int	tcdell	=	16;

static void
ttinit(void)
{
}

static void
tttidy(void)
{
}

static void
ttmove(int row, int col)
{
	if (ttrow!=row || ttcol!=col) {
		ttputc(ESC);
		ttputc('[');
		asciiparm(row+1);
		ttputc(';');
		asciiparm(col+1);
		ttputc('H');
		ttrow = row;
		ttcol = col;
	}
}

static void
tteeol(void)
{
	ttputc(ESC);
	ttputc('[');
	ttputc('K');
}

static void
tteeop(void)
{
	ttputc(ESC);
	ttputc('[');
	ttputc('J');
}

static void
ttbeep(void)
{
	ttputc(BEL);
	ttflush();
}

static void
asciiparm(int n)
{
	register int	q;

	q = n/10;
	if (q != 0)
		asciiparm(q);
	ttputc((n%10) + '0');
}

static void
ttinsl(int row, int bot, int nchunk)
{
	if (row == bot) {
		if (nchunk != 1)
			abort();
		ttmove(row, 0);
		tteeol();
	} else {
		ttwindow(row, bot);
		ttmove(row, 0);
		while (nchunk--) {
			ttputc(ESC);
			ttputc('M');
		}
	}
}

static void
ttdell(int row, int bot, int nchunk)
{
	if (row == bot) {
		if (nchunk != 1)
			abort();
		ttmove(row, 0);
		tteeol();
	} else {
		ttwindow(row, bot);
		ttmove(bot, 0);
		while (nchunk--)
			ttputc(LF);
	}
}

static void
ttwindow(int top, int bot)
{
	if (tttop!=top || ttbot!=bot) {
		ttputc(ESC);
		ttputc('[');
		asciiparm(top+1);
		ttputc(';');
		asciiparm(bot+1);
		ttputc('r');
		ttrow = HUGE;
		ttcol = HUGE;
		tttop = top;
		ttbot = bot;
	}
}

static void
ttnowindow(void)
{
	ttputc(ESC);
	ttputc('[');
	ttputc(';');
	ttputc('r');
	ttrow = HUGE;
	ttcol = HUGE;
	tttop = HUGE;
	ttbot = HUGE;
}

static void
ttcolor(int color)
{
	if (color != tthue) {

		if (color == CTEXT) {
			ttputc(ESC);
			ttputc('[');
			ttputc('m');
		} else if (color == CMODE) {
			ttputc(ESC);
			ttputc('[');
			ttputc('7');
			ttputc('m');
		}

		tthue = color;
	}
}

static volatile sig_atomic_t resized;
static void sigwinch(int s){(void)s;resized=1;}

static void
ttresize(void)
{
	struct winsize ws;
	if (ioctl(0,TIOCGWINSZ,&ws)==0&&ws.ws_row&&ws.ws_col){
		nrow=ws.ws_row>NROW?NROW:ws.ws_row;
		ncol=ws.ws_col>NCOL?NCOL:ws.ws_col;
	}
}
#define	AGRAVE	0x60

static short	lk201map[] = {
	KRANDOM,	KFIND,		KINSERT,	KREMOVE,
	KSELECT,	KPREV,		KNEXT,		KRANDOM,
	KRANDOM,	KRANDOM,	KRANDOM,	KRANDOM,
	KRANDOM,	KRANDOM,	KF4,		KRANDOM,
	KRANDOM,	KF6,		KF7,		KF8,
	KF9,		KF10,		KRANDOM,	KF11,
	KF12,		KF13,		KF14,		KRANDOM,
	KHELP,		KDO,		KRANDOM,	KF17,
	KF18,		KF19,		KF20
};

char	*keystrings[] = {
	NULL,		"Up",		"Down",		"Left",
	"Right",	"Find",		"Insert",	"Remove",
	"Select",	"Previous",	"Next",		"F4",
	"F6",		"F7",		"F8",		"F9",
	"F10",		"F11",		"F12",		"F13",
	"F14",		"Help",		"Do",		"F17",
	"F18",		"F19",		"F20",		"PF1",
	"PF2",		"PF3",		"PF4",		NULL
};

static int
getkbd(void)
{
	register int	c;
	register int	n;
loop:
	c = ttgetc();
	if (c == AGRAVE)
		c = METACH;
	if (c == ESC) {
		{fd_set r;FD_ZERO(&r);FD_SET(0,&r);struct timeval t={0,50000};
		 if(rh>=rt&&!select(1,&r,0,0,&t)){quit(0,0,0);return 0;}}
		c = ttgetc();
		if (c == '[') {
			c = ttgetc();
			if (c == '<') {
				int b=0,x=0,y=0,ch,row; LINE *lp;
				while((ch=ttgetc())!=';') b=b*10+ch-'0';
				while((ch=ttgetc())!=';') x=x*10+ch-'0';
				while((ch=ttgetc())!='M'&&ch!='m') y=y*10+ch-'0';
				if(b>=64&&b<128){if(b&2){/* SGR btn 66/67 = horiz scroll; xterm/mintty only, macOS Terminal/iTerm2 don't send */
				hoff=(b&1)?hoff+4:hoff>4?hoff-4:0;}else{int i;LINE*p=curwp->w_linep;int sk=curwp->w_skip;
				for(i=0;i<3;i++){if(b&1){int wr=wrap_rows(p); if(sk<wr-1)sk++; else{LINE*ln=lforw(p);if(ln!=curbp->b_linep){p=ln;sk=0;}}}
				else{if(sk>0)sk--; else{LINE*ln=lback(p);if(ln!=curbp->b_linep){p=ln;sk=wrap_rows(p)-1;}}}}
				curwp->w_linep=p;curwp->w_skip=sk;curwp->w_dotp=p;curwp->w_doto=0;}curwp->w_flag|=WFHARD;update();goto loop;}
				x--; y--; row=y-curwp->w_toprow;
				if(!nosb&&x>=ncol-2&&y>0&&row>=0&&row<curwp->w_ntrows){int t=0;LINE*p;
					for(p=lforw(curbp->b_linep);p!=curbp->b_linep;p=lforw(p))t+=wrap_rows(p);
					int g=row*t/curwp->w_ntrows,seen=0;LINE*tl=lforw(curbp->b_linep);int tsk=0;
					for(p=lforw(curbp->b_linep);p!=curbp->b_linep;p=lforw(p)){int wr=wrap_rows(p);if(seen+wr>g){tl=p;tsk=g-seen;break;}seen+=wr;}
					curwp->w_linep=curwp->w_dotp=tl;curwp->w_skip=tsk;curwp->w_doto=0;curwp->w_flag|=WFHARD;update();goto loop;}
				if(b&32)goto loop;
				if(y==0&&ch=='M'){if(x>=ncol-3){quit(0,0,0);goto loop;}
				else if(x>=ncol-31&&x<ncol-24){speak_line(0,0,0);goto loop;}
				else if(x>=ncol-23&&x<ncol-17){stop_speak(0,0,0);goto loop;}
				else if(x>=ncol-15&&x<ncol-5){
					char fn[NFILEN]="";FILE*fp;
					eprintf("[Pick a file...]");update();ttflush();
#ifdef __APPLE__
					fp=popen("osascript -e 'tell app \"SystemUIServer\" to activate' -e 'POSIX path of (choose file)' 2>/dev/null","r");
#else
					fp=popen("zenity --file-selection 2>/dev/null || kdialog --getopenfilename . 2>/dev/null","r");
#endif
					if(fp){if(fgets(fn,NFILEN,fp))fn[strcspn(fn,"\n")]=0;pclose(fp);}
					if(fn[0]){readin(fn);sgarbf=TRUE;}else eprintf("[Cancelled]");
					goto loop;}}
				if(row>=0 && row<curwp->w_ntrows) {
					for(lp=curwp->w_linep;row>0&&lp!=curbp->b_linep;row--){if(fold_a&&LSA(lp))FSKIP(lp,curbp);else lp=lforw(lp);}
					if(ch=='M'&&LSA(lp)){fold_a=!fold_a;curwp->w_dotp=lp;curwp->w_doto=0;curwp->w_flag|=WFHARD;sgarbf=TRUE;update();goto loop;}
					curwp->w_dotp=lp;{int i,cc;for(i=cc=0;i<llength(lp)&&cc<x;cc=lgetc(lp,i++)==9?(cc|7)+1:cc+1){}curwp->w_doto=i;}
					if(ch=='M'){if(b>=128&&!(b&32)){backdir(0, 1, KRANDOM);}else if(!(b&3)&&!(b&32)&&dirmode){char f[80];int i,nn=llength(lp);
						for(i=0;i<nn;i++)f[i]=lgetc(lp,i);f[nn]=0;
						if(n>1&&f[0]=='>')filldir(f+2);else{dirmode=0;readin(f+2);}}}
					curwp->w_flag|=WFMOVE; update();
				}
				goto loop;
			}
			if (c == 'A')
				return (KUP);
			if (c == 'B')
				return (KDOWN);
			if (c == 'C')
				return (KRIGHT);
			if (c == 'D')
				return (KLEFT);
			if (c>='0' && c<='9') {
				n = 0;
				do {
					n = 10*n + c - '0';
					c = ttgetc();
				} while (c>='0' && c<='9');
				if (c=='~' && n<=34) {
					c = lk201map[n];
					if (c != KRANDOM)
						return (c);
					goto loop;
				}
				if (c=='~' && (n==200||n==201)) {pmode=(n==200); if(!pmode){update();ttflush();} goto loop;}
			}
			goto loop;
		}
		if (c == 'O') {
			c = ttgetc();
			if (c == 'A')
				return (KUP);
			if (c == 'B')
				return (KDOWN);
			if (c == 'C')
				return (KRIGHT);
			if (c == 'D')
				return (KLEFT);
			if (c == 'P')
				return (KPF1);
			if (c == 'Q')
				return (KPF2);
			if (c == 'R')
				return (KPF3);
			if (c == 'S')
				return (KPF4);
			goto loop;
		}
		if (ISLOWER(c) != FALSE)
			c = TOUPPER(c);
		if (c>=0x00 && c<=0x1F)
			c = KCTRL | (c+'@');
		return (KMETA | c);
	}
	return (c);
}

static void
ttykeymapinit(void)
{
	register SYMBOL	*sp;
	register int	i;

	keydup(KFIND,	"search-again");
	keydup(KHELP,	"help");
	keydup(KINSERT, "yank");
	keydup(KREMOVE, "kill-region");
	keydup(KSELECT, "goto-eob");
	keydup(KPREV,	"back-page");
	keydup(KNEXT,	"forw-page");
	keydup(KDO,	"execute-macro");
	keydup(KF17,	"back-window");
	keydup(KF18,	"forw-window");
	keydup(KF19,	"enlarge-window");
	keydup(KF20,	"shrink-window");
	keydup(KUP,	"back-line");
	keydup(KDOWN,	"forw-line");
	keydup(KRIGHT,	"forw-char");
	keydup(KLEFT,	"back-char");

	if ((sp=symlookup("ins-self")) == NULL)
		abort();
	for (i=0xA0; i<0xFF; ++i) {
		if (i!=0xA4 && i!=0xA6 && i!=0xAC && i!=0xAD && i!=0xAE
		&&  i!=0xAF && i!=0xB4 && i!=0xB8 && i!=0xBE && i!=0xF0
		&&  i!=0xFE && i!=0xA0) {
			if (binding[i] != NULL)
				abort();
			binding[i] = sp;
			++sp->s_nkey;
		}
	}
}
#include    <stdarg.h>
int	epresf	= FALSE;
int	nmsg	= 0;
int	curmsgf	= FALSE;
int	newmsgf	= FALSE;

char	msg[NMSG];

static int
writemsg(char * sp)
{
	register int	c;

	if ((int)(nmsg+strlen(sp)+1) > NMSG)
		return (FALSE);
	while ((c = *sp++) != '\0')
		msg[nmsg++] = c;
	msg[nmsg++] = '\n';
	newmsgf = TRUE;
	return (TRUE);
}

static int
readmsg(int f, int n, int k)
{
	register int	c;
	register int	i;
	register int	j;

	if (nmsg == 0)
		return (TRUE);
	newmsgf = FALSE;
	update();
	ttcolor(CTEXT);
	i = 0;
	while (i < nmsg) {
		ttmove(nrow-1, 0);
		while (i<nmsg && (c=msg[i++])!='\n')
			eputc(c);
		tteeol();
		ttmove(nrow-1, 0);
		ttflush();
		for (;;) {
			c = ttgetc();
			switch (c) {
			case 0x0E:
			case 0x20:
			case 0x0D:
				break;

			case 0x10:
			case 0x08:
				do {
					--i;
				} while (i!=0 && msg[i-1]!='\n');
				if (i != 0) {
					do {
						--i;
					} while (i!=0 && msg[i-1]!='\n');
				}
				break;

			case 0x03:
				j = 0;
				while (i < nmsg)
					msg[j++] = msg[i++];
				nmsg = j;
				eerase();
				return (TRUE);

			case 0x07:
				ttbeep();
				eerase();
				return (ABORT);

			default:
				continue;
			}
			break;
		}
	}
	nmsg = 0;
	eerase();
	return (TRUE);
}

static void
eerase(void)
{
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
	tteeol();
	ttflush();
	epresf = FALSE;
}

static int ereply(char* fp, char* buf, int nbuf, ...);

static int
eyesno(char * sp)
{
	register int	s;
	char		buf[64];

	for (;;) {
		s = ereply("%s [y/n]? ", buf, sizeof(buf), sp);
		if (s == ABORT)
			return (ABORT);
		if (s != FALSE) {
			if (buf[0]=='y' || buf[0]=='Y')
				return (TRUE);
			if (buf[0]=='n' || buf[0]=='N')
				return (FALSE);
		}
	}
}

static int ereply(char* fp, char* buf, int nbuf, ...)
{
    va_list ap;
    int result;
    va_start(ap, nbuf);
	result = eread(fp, buf, nbuf, EFNEW|EFCR, ap);
    va_end(ap);
    return result;
}
static int ereadf(char*fp,char*buf,int n,int f,...){va_list a;int r;va_start(a,f);r=eread(fp,buf,n,f,a);va_end(a);return r;}

static int
eread(char * fp, char * buf, int nbuf, int flag, va_list ap)
{
	register int	cpos;
	register SYMBOL	*sp1;
	register SYMBOL	*sp2;
	register int	i;
	register int	c;
	register int	h;
	register int	nhits;
	register int	nxtra;
	register int	bxtra;

	cpos = 0;
	if (kbdmop != NULL) {
		while ((c = *kbdmop++) != '\0')
			buf[cpos++] = c;
		buf[cpos] = '\0';
		goto done;
	}
	if ((flag&EFNEW)!=0 || ttrow!=nrow-1) {
		ttcolor(CTEXT);
		ttmove(nrow-1, 0);
		epresf = TRUE;
	} else
		eputc(' ');
	eformat(fp, ap);
	tteeol();
	ttflush();
	for (;;) {
		c = ttgetc();
		if (c==' ' && (flag&EFAUTO)!=0) {
			nhits = 0;
			nxtra = HUGE;
			for (h=0; h<NSHASH; ++h) {
				sp1 = symbol[h];
				while (sp1 != NULL) {
					for (i=0; i<cpos; ++i) {
						if (buf[i] != sp1->s_name[i])
							break;
					}
					if (i == cpos) {
						if (nhits == 0)
							sp2 = sp1;
						++nhits;
						bxtra = getxtra(sp1, sp2, cpos);
						if (bxtra < nxtra)
							nxtra = bxtra;
					}
					sp1 = sp1->s_symp;
				}
			}
			if (nhits == 0)
				continue;
			for (i=0; i<nxtra && cpos<nbuf-1; ++i) {
				c = sp2->s_name[cpos];
				buf[cpos++] = c;
				eputc(c);
			}
			ttflush();
			if (nhits != 1)
				continue;
			c = 0x0D;
		}
		switch (c) {
		case 0x0D:
			buf[cpos] = '\0';
			if (kbdmip != NULL) {
				if (kbdmip+cpos+1 > &kbdm[NKBDM-3]) {
					(void) ctrlg(FALSE, 0, KRANDOM);
					ttflush();
					return (ABORT);
				}
				for (i=0; i<cpos; ++i)
					*kbdmip++ = buf[i];
				*kbdmip++ = '\0';
			}
			if ((flag&EFCR) != 0) {
				ttputc(0x0D);
				ttflush();
			}
			goto done;

		case 0x07:
			eputc(0x07);
			(void) ctrlg(FALSE, 0, KRANDOM);
			ttflush();
			return (ABORT);

		case 0x7F:
		case 0x08:
			if (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
				ttflush();
			}
			break;

		case 0x15:
			while (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
			}
			ttflush();
			break;

		default:
			if (cpos < nbuf-1) {
				buf[cpos++] = c;
				eputc(c);
				ttflush();
			}
		}
	}
done:
	if (buf[0] == '\0')
		return (FALSE);
	return (TRUE);
}

static int
getxtra(SYMBOL * sp1, SYMBOL * sp2, int cpos)
{
	register int	i;

	i = cpos;
	for (;;) {
		if (sp1->s_name[i] != sp2->s_name[i])
			break;
		if (sp1->s_name[i] == '\0')
			break;
		++i;
	}
	return (i - cpos);
}

static void eprintf(char* fp, ...)
{
	va_list ap;
	va_start(ap, fp);
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
	eformat(fp, ap);
	tteeol();
	ttflush();
	epresf = TRUE;
	va_end(ap);
}

static void
eformat(char * fp, va_list ap)
{
	register int	c;

	while ((c = *fp++) != '\0') {
		if (c != '%')
			eputc(c);
		else {
			c = *fp++;
			switch (c) {
			case 'd':
				eputi(va_arg(ap, int), 10);
				break;

			case 'o':
				eputi(va_arg(ap, int),  8);
				break;

			case 's':
				eputs(va_arg(ap, char*));
				break;

			default:
				eputc(c);
			}
		}
	}
}

static void
eputi(int i, int r)
{
	register int	q;

	if ((q=i/r) != 0)
		eputi(q, r);
	eputc(i%r+'0');
}

static void
eputs(char * s)
{
	register int	c;

	while ((c = *s++) != '\0')
		eputc(c);
}

static void
eputc(int c)
{
	if (ttcol < ncol) {
		if (ISCTRL(c) != FALSE) {
			eputc('^');
			c ^= 0x40;
		}
		ttputc(c);
		++ttcol;
	}
}
#define	NBLOCK	16

#ifndef	KBLOCK
#define	KBLOCK	256
#endif

static char	*kbufp	= NULL;
static int	kused	= 0;
static int	ksize	= 0;

static LINE *
lalloc(int used)
{
	register LINE	*lp;
	register int	size;

	size = used<NBLOCK ? NBLOCK : used*2;
	if ((lp=(LINE *)malloc(sizeof(LINE)+size)) == NULL) {
		eprintf("Cannot allocate %d bytes", size);
		return (NULL);
	}
	lp->l_size = size;
	lp->l_used = used;
	return (lp);
}

static void
lfree(LINE * lp)
{
	register BUFFER	*bp;
	register WINDOW	*wp;

	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == lp)
			wp->w_linep = lp->l_fp;
		if (wp->w_dotp  == lp) {
			wp->w_dotp  = lp->l_fp;
			wp->w_doto  = 0;
		}
		if (wp->w_markp == lp) {
			wp->w_markp = lp->l_fp;
			wp->w_marko = 0;
		}
		wp = wp->w_wndp;
	}
	bp = bheadp;
	while (bp != NULL) {
		if (bp->b_nwnd == 0) {
			if (bp->b_dotp  == lp) {
				bp->b_dotp = lp->l_fp;
				bp->b_doto = 0;
			}
			if (bp->b_markp == lp) {
				bp->b_markp = lp->l_fp;
				bp->b_marko = 0;
			}
		}
		bp = bp->b_bufp;
	}
	lp->l_bp->l_fp = lp->l_fp;
	lp->l_fp->l_bp = lp->l_bp;
	free((char *) lp);
}

static void
lchange(int flag)
{
	register WINDOW	*wp;

	if (curbp->b_nwnd != 1)
		flag = WFHARD;
	if ((curbp->b_flag&BFCHG) == 0) {
		flag |= WFMODE;
		curbp->b_flag |= BFCHG;
	}
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= flag;
		wp = wp->w_wndp;
	}
}

static int
linsert(int n, int c)
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register int	doto;
	if (ro_flag) return FALSE;
	register int	i;
	register WINDOW	*wp;

	if(!ul)uc[ut++&2047]=c;
	lchange(WFEDIT);
	lp1 = curwp->w_dotp;
	if (lp1 == curbp->b_linep) {
		if (curwp->w_doto != 0) {
			eprintf("bug: linsert");
			return (FALSE);
		}
		if ((lp2=lalloc(n)) == NULL)
			return (FALSE);
		lp3 = lp1->l_bp;
		lp3->l_fp = lp2;
		lp2->l_fp = lp1;
		lp1->l_bp = lp2;
		lp2->l_bp = lp3;
		for (i=0; i<n; ++i)
			lp2->l_text[i] = c;
		curwp->w_dotp = lp2;
		curwp->w_doto = n;
		return (TRUE);
	}
	doto = curwp->w_doto;
	if (lp1->l_used+n > lp1->l_size) {
		if ((lp2=lalloc(lp1->l_used+n)) == NULL)
			return (FALSE);
		cp1 = &lp1->l_text[0];
		cp2 = &lp2->l_text[0];
		while (cp1 != &lp1->l_text[doto])
			*cp2++ = *cp1++;
		cp2 += n;
		while (cp1 != &lp1->l_text[lp1->l_used])
			*cp2++ = *cp1++;
		lp1->l_bp->l_fp = lp2;
		lp2->l_fp = lp1->l_fp;
		lp1->l_fp->l_bp = lp2;
		lp2->l_bp = lp1->l_bp;
		free((char *) lp1);
	} else {
		lp2 = lp1;
		lp2->l_used += n;
		cp2 = &lp1->l_text[lp1->l_used];
		cp1 = cp2-n;
		while (cp1 != &lp1->l_text[doto])
			*--cp2 = *--cp1;
	}
	for (i=0; i<n; ++i)
		lp2->l_text[doto+i] = c;
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			wp->w_dotp = lp2;
			if (wp==curwp || wp->w_doto>doto)
				wp->w_doto += n;
		}
		if (wp->w_markp == lp1) {
			wp->w_markp = lp2;
			if (wp->w_marko > doto)
				wp->w_marko += n;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

static int
lnewline(void)
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register int	doto;
	register WINDOW	*wp;

	if(!ul)uc[ut++&2047]=0;
	lchange(WFHARD);
	lp1  = curwp->w_dotp;
	doto = curwp->w_doto;
	if ((lp2=lalloc(doto)) == NULL)
		return (FALSE);
	cp1 = &lp1->l_text[0];
	cp2 = &lp2->l_text[0];
	while (cp1 != &lp1->l_text[doto])
		*cp2++ = *cp1++;
	cp2 = &lp1->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	lp1->l_used -= doto;
	lp2->l_bp = lp1->l_bp;
	lp1->l_bp = lp2;
	lp2->l_bp->l_fp = lp2;
	lp2->l_fp = lp1;
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep == lp1)
			wp->w_linep = lp2;
		if (wp->w_dotp == lp1) {
			if (wp->w_doto < doto)
				wp->w_dotp = lp2;
			else
				wp->w_doto -= doto;
		}
		if (wp->w_markp == lp1) {
			if (wp->w_marko < doto)
				wp->w_markp = lp2;
			else
				wp->w_marko -= doto;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

static int
ldelete(int n, int kflag)
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*dotp;
	register int	doto;
	register int	chunk;
	if (ro_flag) return FALSE;
	register WINDOW	*wp;

	while (n != 0) {
		dotp = curwp->w_dotp;
		doto = curwp->w_doto;
		if (dotp == curbp->b_linep)
			return (FALSE);
		chunk = dotp->l_used-doto;
		if (chunk > n)
			chunk = n;
		if (chunk == 0) {
			if(!ul)uc[ut++&2047]=-256;
			lchange(WFHARD);
			if (ldelnewline() == FALSE
			|| (kflag!=FALSE && kinsert('\n')==FALSE))
				return (FALSE);
			--n;
			continue;
		}
		lchange(WFEDIT);
		if(!ul){int i_;for(i_=0;i_<chunk;i_++)uc[ut++&2047]=-lgetc(dotp,doto+i_);}
		cp1 = &dotp->l_text[doto];
		cp2 = cp1 + chunk;
		if (kflag != FALSE) {
			while (cp1 != cp2) {
				if (kinsert(*cp1) == FALSE)
					return (FALSE);
				++cp1;
			}
			cp1 = &dotp->l_text[doto];
		}
		while (cp2 != &dotp->l_text[dotp->l_used])
			*cp1++ = *cp2++;
		dotp->l_used -= chunk;
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_dotp==dotp && wp->w_doto>=doto) {
				wp->w_doto -= chunk;
				if (wp->w_doto < doto)
					wp->w_doto = doto;
			}
			if (wp->w_markp==dotp && wp->w_marko>=doto) {
				wp->w_marko -= chunk;
				if (wp->w_marko < doto)
					wp->w_marko = doto;
			}
			wp = wp->w_wndp;
		}
		n -= chunk;
	}
	return (TRUE);
}

static int
ldelnewline(void)
{
	register char	*cp1;
	register char	*cp2;
	register LINE	*lp1;
	register LINE	*lp2;
	register LINE	*lp3;
	register WINDOW	*wp;

	lp1 = curwp->w_dotp;
	lp2 = lp1->l_fp;
	if (lp2 == curbp->b_linep) {
		if (lp1->l_used == 0)
			lfree(lp1);
		return (TRUE);
	}
	if (lp2->l_used <= lp1->l_size-lp1->l_used) {
		cp1 = &lp1->l_text[lp1->l_used];
		cp2 = &lp2->l_text[0];
		while (cp2 != &lp2->l_text[lp2->l_used])
			*cp1++ = *cp2++;
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_linep == lp2)
				wp->w_linep = lp1;
			if (wp->w_dotp == lp2) {
				wp->w_dotp  = lp1;
				wp->w_doto += lp1->l_used;
			}
			if (wp->w_markp == lp2) {
				wp->w_markp  = lp1;
				wp->w_marko += lp1->l_used;
			}
			wp = wp->w_wndp;
		}
		lp1->l_used += lp2->l_used;
		lp1->l_fp = lp2->l_fp;
		lp2->l_fp->l_bp = lp1;
		free((char *) lp2);
		return (TRUE);
	}
	if ((lp3=lalloc(lp1->l_used+lp2->l_used)) == NULL)
		return (FALSE);
	cp1 = &lp1->l_text[0];
	cp2 = &lp3->l_text[0];
	while (cp1 != &lp1->l_text[lp1->l_used])
		*cp2++ = *cp1++;
	cp1 = &lp2->l_text[0];
	while (cp1 != &lp2->l_text[lp2->l_used])
		*cp2++ = *cp1++;
	lp1->l_bp->l_fp = lp3;
	lp3->l_fp = lp2->l_fp;
	lp2->l_fp->l_bp = lp3;
	lp3->l_bp = lp1->l_bp;
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_linep==lp1 || wp->w_linep==lp2)
			wp->w_linep = lp3;
		if (wp->w_dotp == lp1)
			wp->w_dotp  = lp3;
		else if (wp->w_dotp == lp2) {
			wp->w_dotp  = lp3;
			wp->w_doto += lp1->l_used;
		}
		if (wp->w_markp == lp1)
			wp->w_markp  = lp3;
		else if (wp->w_markp == lp2) {
			wp->w_markp  = lp3;
			wp->w_marko += lp1->l_used;
		}
		wp = wp->w_wndp;
	}
	free((char *) lp1);
	free((char *) lp2);
	return (TRUE);
}

static int
lreplace(int plen, char * st, int f)
{
	register int	rlen;
	register int	rtype;
	register int	c;
	register int	doto;

	backchar(TRUE, plen, KRANDOM);
	rtype = _L;
	c = lgetc(curwp->w_dotp, curwp->w_doto);
	if (ISUPPER(c)!=FALSE  &&  f==FALSE) {
		rtype = _U|_L;
		if (curwp->w_doto+1 < llength(curwp->w_dotp)) {
			c = lgetc(curwp->w_dotp, curwp->w_doto+1);
			if (ISUPPER(c) != FALSE) {
				rtype = _U;
			}
		}
	}

	rlen = strlen(st);
	doto = curwp->w_doto;
	if (plen > rlen)
		ldelete(plen-rlen, FALSE);
	else if (plen < rlen) {
		if (linsert(rlen-plen, ' ') == FALSE)
			return (FALSE);
	}
	curwp->w_doto = doto;

	while ((c = *st++&0xff) != '\0') {
		if ((rtype&_U)!=0  &&  ISLOWER(c)!=0)
			c = TOUPPER(c);
		if (rtype == (_U|_L))
			rtype = _L;
		if (c == '\n') {
			if (curwp->w_doto == llength(curwp->w_dotp))
				forwchar(FALSE, 1, KRANDOM);
			else {
				ldelete(1, FALSE);
				lnewline();
			}
		} else if (curwp->w_dotp == curbp->b_linep) {
			linsert(1, c);
		} else if (curwp->w_doto == llength(curwp->w_dotp)) {
			ldelete(1, FALSE);
			linsert(1, c);
		} else
			lputc(curwp->w_dotp, curwp->w_doto++, c);
	}
	lchange(WFHARD);
	return (TRUE);
}

static void
kdelete(void)
{
	if (kbufp != NULL) {
		free((char *) kbufp);
		kbufp = NULL;
		kused = 0;
		ksize = 0;
	}
}

static int
kinsert(int c)
{
	register char	*nbufp;
	register int	i;

	if (kused == ksize) {
		if ((nbufp=malloc(ksize+KBLOCK)) == NULL) {
			eprintf("Too many kills");
			return (FALSE);
		}
		for (i=0; i<ksize; ++i)
			nbufp[i] = kbufp[i];
		if (kbufp != NULL)
			free((char *) kbufp);
		kbufp  = nbufp;
		ksize += KBLOCK;
	}
	kbufp[kused++] = c;
	return (TRUE);
}

static int
kremove(int n)
{
	if (n >= kused)
		return (-1);
	return (kbufp[n] & 0xFF);
}
#define	DIRLIST	0

static int	ctrlg(int, int, int);
static int	quit(int, int, int);
static int	ctlxlp(int, int, int);
static int	ctlxrp(int, int, int);
static int	ctlxe(int, int, int);
static int	jeffexit(int, int, int);
static int	undo(int, int, int);
static int	showversion(int, int, int);

static int	forwsearch(int, int, int);
static int	backsearch(int, int, int);
static int	searchagain(int, int, int);
static int	forwisearch(int, int, int);
static int	backisearch(int, int, int);
static int	queryrepl(int, int, int);

static int	gotobol(int, int, int);
static int	backchar(int, int, int);
static int	gotoeol(int, int, int);
static int	forwchar(int, int, int);
static int	gotobob(int, int, int);
static int	gotoeob(int, int, int);
static int	forwline(int, int, int);
static int	backline(int, int, int);
static int	forwpage(int, int, int);
static int	backpage(int, int, int);
static int	selectall(int, int, int);
static int	setmark(int, int, int);
static int	swapmark(int, int, int);
static int	gotoline(int, int, int);

static int	listbuffers(int, int, int);
static int	usebuffer(int, int, int);
static int	killbuffer(int, int, int);

#if	DIRLIST

static int	dirlist(int, int, int);
#endif

static int	readmsg(int, int, int);

static int	fileread(int, int, int);
static int	filevisit(int, int, int);
static int	filewrite(int, int, int);
static int	filesave(int, int, int);
static int	filename(int, int, int);

static int	selfinsert(int, int, int);
static int	showcpos(int, int, int);
static int	twiddle(int, int, int);
static int	quote(int, int, int);
static int	openline(int, int, int);
static int	newline(int, int, int);
static int	deblank(int, int, int);
static int	indent(int, int, int);
static int	forwdel(int, int, int);
static int	backdel(int, int, int);
static int	killline(int, int, int);
static int	yank(int, int, int);

static void	clip_osc52(void);
static int	killregion(int, int, int);
static int	copyregion(int, int, int);
static int	lowerregion(int, int, int);
static int	upperregion(int, int, int);

static int	spawncli(int, int, int);
static int	speak_line(int, int, int);
static int	stop_speak(int, int, int);
static int	speak_running(void);

static int	reposition(int, int, int);
static int	refresh(int, int, int);
static int	nextwind(int, int, int);
static int	prevwind(int, int, int);
static int	mvdnwind(int, int, int);
static int	mvupwind(int, int, int);
static int	onlywind(int, int, int);
static int	splitwind(int, int, int);
static int	enlargewind(int, int, int);
static int	shrinkwind(int, int, int);

static int	backword(int, int, int);
static int	forwword(int, int, int);
static int	upperword(int, int, int);
static int	lowerword(int, int, int);
static int	capword(int, int, int);
static int	delfword(int, int, int);
static int	delbword(int, int, int);

static int	extend(int, int, int);
static int	help(int, int, int);
static int	bindtokey(int, int, int);
static int	wallchart(int, int, int);
static int	backdir(int, int, int);

typedef	struct	{
	short	k_key;
	int	(*k_funcp)(int, int, int);
	char	*k_name;
}	KEY;

KEY	key[] = {
	KCTRL|'A',	selectall,	"select-all",
	KCTRL|'B',	backdir,	"back-dir",
	KCTRL|'C',	copyregion,	"copy-region",
	KCTRL|'D',	quit,		"quit",
	KCTRL|'E',	gotoeol,	"goto-eol",
	KCTRL|'F',	forwisearch,	"forw-i-search",
	KCTRL|'G',	gotoline,	"goto-line",
	KCTRL|'H',	queryrepl,	"query-replace",
	KCTRL|'I',	selfinsert,	"ins-self",
	KCTRL|'J',	indent,		"ins-nl-and-indent",
	KCTRL|'K',	killline,	"kill-line",
	KCTRL|'L',	refresh,	"refresh",
	KCTRL|'M',	newline,	"ins-nl",
	KCTRL|'N',	forwline,	"forw-line",
	KCTRL|'O',	filevisit,	"file-visit",
	KCTRL|'P',	backline,	"back-line",
	KCTRL|'Q',	quit,		"quit",
	KCTRL|'R',	backisearch,	"back-i-search",
	KCTRL|'S',	filesave,	"file-save",
	KCTRL|'T',	speak_line,	"speak-line",
	KCTRL|'Y',	stop_speak,	"stop-speak",
	KCTRL|'V',	yank,		"yank",
	KCTRL|'W',	quit,		"quit",
	KCTRL|'X',	killregion,	"kill-region",
	KCTRL|'Z',	undo,		"undo",
	KCTRL|'\\',	splitwind,	"split-window",
	KCTRL|'@',	setmark,	"set-mark",
	-1,		backchar,	"back-char",
	-1,		forwchar,	"forw-char",
	-1,		ctrlg,		"abort",
	-1,		backdel,	"back-del-char",
	-1,		openline,	"ins-nl-and-backup",
	-1,		quote,		"quote",
	-1,		twiddle,	"twiddle",
	-1,		forwpage,	"forw-page",
	-1,		backpage,	"back-page",
	-1,		listbuffers,	"display-buffers",
	-1,		filename,	"set-file-name",
	-1,		lowerregion,	"lower-region",
	-1,		upperregion,	"upper-region",
	-1,		fileread,	"file-read",
	-1,		filewrite,	"file-write",
	-1,		swapmark,	"swap-dot-and-mark",
	-1,		showcpos,	"display-position",
	-1,		ctlxlp,		"start-macro",
	-1,		ctlxrp,		"end-macro",
	-1,		onlywind,	"only-window",
	-1,		usebuffer,	"use-buffer",
	-1,		ctlxe,		"execute-macro",
	-1,		nextwind,	"forw-window",
	-1,		prevwind,	"back-window",
	-1,		enlargewind,	"enlarge-window",
	-1,		shrinkwind,	"shrink-window",
	-1,		mvdnwind,	"down-window",
	-1,		mvupwind,	"up-window",
	-1,		deblank,	"del-blank-lines",
	-1,		delbword,	"back-del-word",
	-1,		delfword,	"forw-del-word",
	-1,		readmsg,	"display-message",
	-1,		showversion,	"display-version",
	-1,		reposition,	"reposition-window",
	-1,		gotoeob,	"goto-eob",
	-1,		gotobob,	"goto-bob",
	-1,		backword,	"back-word",
	-1,		forwword,	"forw-word",
	-1,		capword,	"cap-word",
	-1,		lowerword,	"lower-word",
	-1,		upperword,	"upper-word",
	-1,		backsearch,	"back-search",
	-1,		forwsearch,	"forw-search",
	-1,		extend,		"extended-command",
	-1,		searchagain,	"search-again",
	-1,		spawncli,	"spawn-cli",
	-1,		help,		"help",
	-1,		wallchart,	"display-bindings",
	-1,		bindtokey,	"bind-to-key"
};

#define	NKEY	(sizeof(key) / sizeof(key[0]))

/*
 * Symbol table lookup.
 * Return a pointer to the SYMBOL node, or NULL if
 * the symbol is not found.
 */
static SYMBOL *
symlookup(char * cp)
{
	register SYMBOL	*sp;

	sp = symbol[symhash(cp)];
	while (sp != NULL) {
		if (strcmp(cp, sp->s_name) == 0)
			return (sp);
		sp = sp->s_symp;
	}
	return (NULL);
}

/*
 * Take a string, and compute the symbol table
 * bucket number. This is done by adding all of the characters
 * together, and taking the sum mod NSHASH. The string probably
 * should not contain any GR characters; if it does the "*cp"
 * may get a nagative number on some machines, and the "%"
 * will return a negative number!
 */
static int
symhash(char * cp)
{
	register int	c;
	register int	n;

	n = 0;
	while ((c = *cp++) != 0)
		n += c;
	return (n % NSHASH);
}

/*
 * Build initial keymap. VSCode-style bindings.
 */
static void
keymapinit(void)
{
	register SYMBOL	*sp;
	register KEY	*kp;
	register int	i;

	for (i=0; i<NKEYS; ++i)
		binding[i] = NULL;
	for (kp = &key[0]; kp < &key[NKEY]; ++kp)
		keyadd(kp->k_key, kp->k_funcp, kp->k_name);
	keydup(0x7F,		"back-del-char");
	keydup(KCTRL|'[',	"abort");
	if ((sp=symlookup("ins-self")) == NULL)
		abort();
	for (i=0x20; i<0x7F; ++i) {
		if (binding[i] != NULL)
			abort();
		binding[i] = sp;
		++sp->s_nkey;
	}
	ttykeymapinit();
}

/*
 * Create a new builtin function "name"
 * with function "funcp". If the "new" is a real
 * key, bind it as a side effect. All errors
 * are fatal.
 */
static void
keyadd(int new, int (*funcp)(int, int, int), char * name)
{
	register SYMBOL	*sp;
	register int	hash;

	if ((sp=(SYMBOL *)malloc(sizeof(SYMBOL))) == NULL)
		abort();
	hash = symhash(name);
	sp->s_symp = symbol[hash];
	symbol[hash] = sp;
	sp->s_nkey = 0;
	sp->s_name = name;
	sp->s_funcp = funcp;
	if (new >= 0) {				/* Bind this key.	*/
		if (binding[new] != NULL)
			abort();
		binding[new] = sp;
		++sp->s_nkey;
	}
}

/*
 * Bind key "new" to the existing
 * routine "name". If the name cannot be found,
 * or the key is already bound, abort.
 */
static void
keydup(int new, char * name)
{
	register SYMBOL	*sp;

	if (binding[new]!=NULL || (sp=symlookup(name))==NULL)
		abort();
	binding[new] = sp;
	++sp->s_nkey;
}
/*
 * Attach a buffer to a window. The
 * values of dot and mark come from the buffer
 * if the use count is 0. Otherwise, they come
 * from some other window.
 */
static int
usebuffer(int f, int n, int k)
{
	register BUFFER	*bp;
	register WINDOW	*wp;
	register int	s;
	char		bufn[NBUFN];

	if ((s=ereply("Use buffer: ", bufn, NBUFN)) != TRUE)
		return (s);
	if ((bp=bfind(bufn, TRUE)) == NULL)
		return (FALSE);
	if (--curbp->b_nwnd == 0) {		/* Last use.		*/
		curbp->b_dotp  = curwp->w_dotp;
		curbp->b_doto  = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;				/* Switch.		*/
	curwp->w_bufp  = bp;
	curwp->w_linep = bp->b_linep;		/* For macros, ignored.	*/
	curwp->w_flag |= WFMODE|WFFORCE|WFHARD;	/* Quite nasty.		*/
	if (bp->b_nwnd++ == 0) {		/* First use.		*/
		curwp->w_dotp  = bp->b_dotp;
		curwp->w_doto  = bp->b_doto;
		curwp->w_markp = bp->b_markp;
		curwp->w_marko = bp->b_marko;
		return (TRUE);
	}
	wp = wheadp;				/* Look for old.	*/
	while (wp != NULL) {
		if (wp!=curwp && wp->w_bufp==bp) {
			curwp->w_dotp  = wp->w_dotp;
			curwp->w_doto  = wp->w_doto;
			curwp->w_markp = wp->w_markp;
			curwp->w_marko = wp->w_marko;
			break;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

/*
 * Dispose of a buffer, by name.
 * Ask for the name. Look it up (don't get too
 * upset if it isn't there at all!). Get quite upset
 * if the buffer is being displayed. Clear the buffer (ask
 * if the buffer has been changed). Then free the header
 * line and the buffer header. Bound to "C-X K".
 */
static int
killbuffer(int f, int n, int k)
{
	register BUFFER	*bp;
	register BUFFER	*bp1;
	register BUFFER	*bp2;
	register int	s;
	char		bufn[NBUFN];

	if ((s=ereply("Kill buffer: ", bufn, NBUFN)) != TRUE)
		return (s);
	if ((bp=bfind(bufn, FALSE)) == NULL)	/* Easy if unknown.	*/
		return (TRUE);
	if (bp->b_nwnd != 0) {			/* Error if on screen.	*/
		eprintf("Buffer is being displayed");
		return (FALSE);
	}
	if ((s=bclear(bp)) != TRUE)		/* Blow text away.	*/
		return (s);
	free((char *) bp->b_linep);		/* Release header line.	*/
	bp1 = NULL;				/* Find the header.	*/
	bp2 = bheadp;
	while (bp2 != bp) {
		bp1 = bp2;
		bp2 = bp2->b_bufp;
	}
	bp2 = bp2->b_bufp;			/* Next one in chain.	*/
	if (bp1 == NULL)			/* Unlink it.		*/
		bheadp = bp2;
	else
		bp1->b_bufp = bp2;
	free((char *) bp);			/* Release buffer block	*/
	return (TRUE);
}

/*
 * Display the buffer list. This is done
 * in two parts. The "makelist" routine figures out
 * the text, and puts it in the buffer whoses header is
 * pointed to by the external "blistp". The "popblist"
 * then pops the data onto the screen. Bound to
 * "C-X C-B".
 */
static int
listbuffers(int f, int n, int k)
{
	register int	s;

	if ((s=makelist()) != TRUE)
		return (s);
	return (popblist());
}

/*
 * Pop the special buffer whose
 * buffer header is pointed to by the external
 * variable "blistp" onto the screen. This is used
 * by the "listbuffers" routine (above) and by
 * some other packages. Returns a status.
 */
static int
popblist(void)
{
	register WINDOW	*wp;
	register BUFFER	*bp;

	if (blistp->b_nwnd == 0) {		/* Not on screen yet.	*/
		if ((wp=wpopup()) == NULL)
			return (FALSE);
		bp = wp->w_bufp;
		if (--bp->b_nwnd == 0) {
			bp->b_dotp  = wp->w_dotp;
			bp->b_doto  = wp->w_doto;
			bp->b_markp = wp->w_markp;
			bp->b_marko = wp->w_marko;
		}
		wp->w_bufp  = blistp;
		++blistp->b_nwnd;
	}
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == blistp) {
			wp->w_linep = lforw(blistp->b_linep);
			wp->w_dotp  = lforw(blistp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
			wp->w_flag |= WFMODE|WFHARD;
		}
		wp = wp->w_wndp;
	}
	return (TRUE);
}

/*
 * This routine rebuilds the
 * text in the special secret buffer
 * that holds the buffer list. It is called
 * by the list buffers command. Return TRUE
 * if everything works. Return FALSE if there
 * is an error (if there is no memory).
 */
static int
makelist(void)
{
	register char	*cp1;
	register char	*cp2;
	register int	c;
	register BUFFER	*bp;
	register LINE	*lp;
	register int	nbytes;
	register int	s;
	char		b[6+1];
	char		line[128];

	blistp->b_flag &= ~BFCHG;		/* Blow away old.	*/
	if ((s=bclear(blistp)) != TRUE)
		return (s);
	strcpy(blistp->b_fname, "");
	if (addline("C   Size Buffer           File") == FALSE
	||  addline("-   ---- ------           ----") == FALSE)
		return (FALSE);
	bp = bheadp;				/* For all buffers	*/
	while (bp != NULL) {
		cp1 = &line[0];			/* Start at left edge	*/
		if ((bp->b_flag&BFCHG) != 0)	/* "*" if changed	*/
			*cp1++ = '*';
		else
			*cp1++ = ' ';
		*cp1++ = ' ';			/* Gap.			*/
		nbytes = 0;			/* Count bytes in buf.	*/
		lp = lforw(bp->b_linep);
		while (lp != bp->b_linep) {
			nbytes += llength(lp)+1;
			lp = lforw(lp);
		}
		itoa_(b, 6, nbytes);		/* 6 digit buffer size.	*/
		cp2 = &b[0];
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		*cp1++ = ' ';			/* Gap.			*/
		cp2 = &bp->b_bname[0];		/* Buffer name		*/
		while ((c = *cp2++) != 0)
			*cp1++ = c;
		cp2 = &bp->b_fname[0];		/* File name		*/
		if (*cp2 != 0) {
			while (cp1 < &line[1+1+6+1+NBUFN+1])
				*cp1++ = ' ';
			while ((c = *cp2++) != 0) {
				if (cp1 < &line[128-1])
					*cp1++ = c;
			}
		}
		*cp1 = 0;			/* Add to the buffer.	*/
		if (addline(line) == FALSE)
			return (FALSE);
		bp = bp->b_bufp;
	}
	return (TRUE);				/* All done		*/
}

/*
 * Used above.
 */
static void
itoa_(char * buf, int width, int num)
{
	buf[width] = 0;				/* End of string.	*/
	while (num >= 10) {			/* Conditional digits.	*/
		buf[--width] = (num%10) + '0';
		num /= 10;
	}
	buf[--width] = num + '0';		/* Always 1 digit.	*/
	while (width != 0)			/* Pad with blanks.	*/
		buf[--width] = ' ';
}

/*
 * The argument "text" points to
 * a string. Append this line to the
 * buffer list buffer. Handcraft the EOL
 * on the end. Return TRUE if it worked and
 * FALSE if you ran out of room.
 */
static int
addline(char * text)
{
	register LINE	*lp;
	register int	i;
	register int	ntext;

	ntext = strlen(text);
	if ((lp=lalloc(ntext)) == NULL)
		return (FALSE);
	for (i=0; i<ntext; ++i)
		lputc(lp, i, text[i]);
	blistp->b_linep->l_bp->l_fp = lp;	/* Hook onto the end	*/
	lp->l_bp = blistp->b_linep->l_bp;
	blistp->b_linep->l_bp = lp;
	lp->l_fp = blistp->b_linep;
	if (blistp->b_dotp == blistp->b_linep)	/* If "." is at the end	*/
		blistp->b_dotp = lp;		/* move it to new line	*/
	return (TRUE);
}

/*
 * Look through the list of
 * buffers. Return TRUE if there
 * are any changed buffers. Special buffers
 * like the buffer list buffer don't count, as
 * they are not in the list. Return FALSE if
 * there are no changed buffers.
 */
static int
anycb(void)
{
	register BUFFER	*bp;

	bp = bheadp;
	while (bp != NULL) {
		if ((bp->b_flag&BFCHG) != 0)
			return (TRUE);
		bp = bp->b_bufp;
	}
	return (FALSE);
}

static BUFFER *
bfind(char * bname, int cflag)
{
	register BUFFER	*bp;

	bp = bheadp;
	while (bp != NULL) {
		if (strcmp(bname, bp->b_bname) == 0)
			return (bp);
		bp = bp->b_bufp;
	}
	if (cflag!=FALSE && (bp=bcreate(bname))!=NULL) {
		bp->b_bufp = bheadp;
		bheadp = bp;
	}
	return (bp);
}

static BUFFER *
bcreate(char * bname)
{
	register BUFFER	*bp;
	register LINE	*lp;

	if ((bp=(BUFFER *)malloc(sizeof(BUFFER))) == NULL)
		return (NULL);
	if ((lp=lalloc(0)) == NULL) {
		free((char *) bp);
		return (NULL);
	}
	bp->b_bufp  = NULL;
	bp->b_dotp  = lp;
	bp->b_doto  = 0;
	bp->b_markp = NULL;
	bp->b_marko = 0;
	bp->b_flag  = 0;
	bp->b_nwnd  = 0;
	bp->b_linep = lp;
	strcpy(bp->b_fname, "");
	strcpy(bp->b_bname, bname);
	lp->l_fp = lp;
	lp->l_bp = lp;
	return (bp);
}

static int
bclear(BUFFER * bp)
{
	register LINE	*lp;
	register int	s;

	if ((bp->b_flag&BFCHG) != 0
	&& (s=eyesno("Discard changes")) != TRUE)
		return (s);
	bp->b_flag  &= ~BFCHG;
	while ((lp=lforw(bp->b_linep)) != bp->b_linep)
		lfree(lp);
	bp->b_dotp  = bp->b_linep;
	bp->b_doto  = 0;
	bp->b_markp = NULL;
	bp->b_marko = 0;
	return (TRUE);
}

static int
reposition(int f, int n, int k)
{
	curwp->w_force = n;
	curwp->w_flag |= WFFORCE;
	return (TRUE);
}

static int
refresh(int f, int n, int k)
{
	register WINDOW	*wp;
	register int	oldnrow;
	register int	oldncol;

	oldnrow = nrow;
	oldncol = ncol;
	ttresize();
	if (nrow!=oldnrow || ncol!=oldncol) {
		wp = wheadp;
		while (wp->w_wndp != NULL)
			wp = wp->w_wndp;
		if (nrow < wp->w_toprow+3) {
			eprintf("Display unusable");
			return (FALSE);
		}
		wp->w_ntrows = nrow-wp->w_toprow-2;
		wp = wheadp;
		while (wp != NULL) {
			wp->w_flag |= WFMODE|WFHARD;
			wp = wp->w_wndp;
		}
		sgarbf = TRUE;
		update();
		eprintf("[New size %d by %d]", nrow, ncol);
	} else
		sgarbf = TRUE;
	return (TRUE);
}

static int
nextwind(int f, int n, int k)
{
	register WINDOW	*wp;

	if ((wp=curwp->w_wndp) == NULL)
		wp = wheadp;
	curwp = wp;
	curbp = wp->w_bufp;
	return (TRUE);
}

static int
prevwind(int f, int n, int k)
{
	register WINDOW	*wp1;
	register WINDOW	*wp2;

	wp1 = wheadp;
	wp2 = curwp;
	if (wp1 == wp2)
		wp2 = NULL;
	while (wp1->w_wndp != wp2)
		wp1 = wp1->w_wndp;
	curwp = wp1;
	curbp = wp1->w_bufp;
	return (TRUE);
}

static int
mvdnwind(int f, int n, int k)
{
	return (mvupwind(f, -n, KRANDOM));
}

static int
mvupwind(int f, int n, int k)
{
	register LINE	*lp;
	register int	i;

	lp = curwp->w_linep;
	if (n < 0) {
		while (n++ && lp!=curbp->b_linep)
			lp = lforw(lp);
	} else {
		while (n-- && lback(lp)!=curbp->b_linep)
			lp = lback(lp);
	}
	curwp->w_linep = lp;
	curwp->w_flag |= WFHARD;
	for (i=0; i<curwp->w_ntrows; ++i) {
		if (lp == curwp->w_dotp)
			return (TRUE);
		if (lp == curbp->b_linep)
			break;
		lp = lforw(lp);
	}
	lp = curwp->w_linep;
	i  = curwp->w_ntrows/2;
	while (i-- && lp!=curbp->b_linep)
		lp = lforw(lp);
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	return (TRUE);
}

static int
onlywind(int f, int n, int k)
{
	register WINDOW	*wp;
	register LINE	*lp;
	register int	i;

	while (wheadp != curwp) {
		wp = wheadp;
		wheadp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			wp->w_bufp->b_markp = wp->w_markp;
			wp->w_bufp->b_marko = wp->w_marko;
		}
		free((char *) wp);
	}
	while (curwp->w_wndp != NULL) {
		wp = curwp->w_wndp;
		curwp->w_wndp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			wp->w_bufp->b_markp = wp->w_markp;
			wp->w_bufp->b_marko = wp->w_marko;
		}
		free((char *) wp);
	}
	lp = curwp->w_linep;
	i  = curwp->w_toprow;
	while (i!=0 && lback(lp)!=curbp->b_linep) {
		--i;
		lp = lback(lp);
	}
	curwp->w_toprow = 0;
	curwp->w_ntrows = nrow-2;
	curwp->w_linep  = lp;
	curwp->w_flag  |= WFMODE|WFHARD;
	return (TRUE);
}

static int
splitwind(int f, int n, int k)
{
	register WINDOW	*wp;
	register LINE	*lp;
	register int	ntru;
	register int	ntrl;
	register int	ntrd;
	register WINDOW	*wp1;
	register WINDOW	*wp2;

	if (curwp->w_ntrows < 3) {
		eprintf("Cannot split a %d line window", curwp->w_ntrows);
		return (FALSE);
	}
	if ((wp = (WINDOW *) malloc(sizeof(WINDOW))) == NULL) {
		eprintf("Cannot allocate WINDOW block");
		return (FALSE);
	}
	++curbp->b_nwnd;
	wp->w_bufp  = curbp;
	wp->w_dotp  = curwp->w_dotp;
	wp->w_doto  = curwp->w_doto;
	wp->w_markp = curwp->w_markp;
	wp->w_marko = curwp->w_marko;
	wp->w_flag  = 0;
	wp->w_force = 0;
	ntru = (curwp->w_ntrows-1) / 2;
	ntrl = (curwp->w_ntrows-1) - ntru;
	lp = curwp->w_linep;
	ntrd = 0;
	while (lp != curwp->w_dotp) {
		++ntrd;
		lp = lforw(lp);
	}
	lp = curwp->w_linep;
	if (ntrd <= ntru) {
		if (ntrd == ntru)
			lp = lforw(lp);
		curwp->w_ntrows = ntru;
		wp->w_wndp = curwp->w_wndp;
		curwp->w_wndp = wp;
		wp->w_toprow = curwp->w_toprow+ntru+1;
		wp->w_ntrows = ntrl;
	} else {
		wp1 = NULL;
		wp2 = wheadp;
		while (wp2 != curwp) {
			wp1 = wp2;
			wp2 = wp2->w_wndp;
		}
		if (wp1 == NULL)
			wheadp = wp;
		else
			wp1->w_wndp = wp;
		wp->w_wndp   = curwp;
		wp->w_toprow = curwp->w_toprow;
		wp->w_ntrows = ntru;
		++ntru;
		curwp->w_toprow += ntru;
		curwp->w_ntrows  = ntrl;
		while (ntru--)
			lp = lforw(lp);
	}
	curwp->w_linep = lp;
	wp->w_linep = lp;
	curwp->w_flag |= WFMODE|WFHARD;
	wp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

static int
enlargewind(int f, int n, int k)
{
	register WINDOW	*adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (shrinkwind(f, -n, KRANDOM));
	if (wheadp->w_wndp == NULL) {
		eprintf("Only one window");
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if (adjwp->w_ntrows <= n) {
		eprintf("Impossible change");
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {
		lp = adjwp->w_linep;
		for (i=0; i<n && lp!=adjwp->w_bufp->b_linep; ++i)
			lp = lforw(lp);
		adjwp->w_linep  = lp;
		adjwp->w_toprow += n;
	} else {
		lp = curwp->w_linep;
		for (i=0; i<n && lback(lp)!=curbp->b_linep; ++i)
			lp = lback(lp);
		curwp->w_linep  = lp;
		curwp->w_toprow -= n;
	}
	curwp->w_ntrows += n;
	adjwp->w_ntrows -= n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

static int
shrinkwind(int f, int n, int k)
{
	register WINDOW	*adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (enlargewind(f, -n, KRANDOM));
	if (wheadp->w_wndp == NULL) {
		eprintf("Only one window");
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if (curwp->w_ntrows <= n) {
		eprintf("Impossible change");
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {
		lp = adjwp->w_linep;
		for (i=0; i<n && lback(lp)!=adjwp->w_bufp->b_linep; ++i)
			lp = lback(lp);
		adjwp->w_linep  = lp;
		adjwp->w_toprow -= n;
	} else {
		lp = curwp->w_linep;
		for (i=0; i<n && lp!=curbp->b_linep; ++i)
			lp = lforw(lp);
		curwp->w_linep  = lp;
		curwp->w_toprow += n;
	}
	curwp->w_ntrows -= n;
	adjwp->w_ntrows += n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

static WINDOW *
wpopup(void)
{
	register WINDOW	*wp;

	if (wheadp->w_wndp == NULL
	&& splitwind(FALSE, 0, KRANDOM) == FALSE)
		return (NULL);
	wp = wheadp;
	while (wp!=NULL && wp==curwp)
		wp = wp->w_wndp;
	return (wp);
}

static int
fileread(int f, int n, int k)
{
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Read file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	return (readin(fname));
}

static int
filevisit(int f, int n, int k)
{
	register BUFFER	*bp;
	register WINDOW	*wp;
	register LINE	*lp;
	register int	i;
	register int	s;
	char		bname[NBUFN];
	char		fname[NFILEN];

	if ((s=ereply("Visit file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
		if (strcmp(bp->b_fname, fname) == 0) {
			if (--curbp->b_nwnd == 0) {
				curbp->b_dotp  = curwp->w_dotp;
				curbp->b_doto  = curwp->w_doto;
				curbp->b_markp = curwp->w_markp;
				curbp->b_marko = curwp->w_marko;
			}
			curbp = bp;
			curwp->w_bufp  = bp;
			if (bp->b_nwnd++ == 0) {
				curwp->w_dotp  = bp->b_dotp;
				curwp->w_doto  = bp->b_doto;
				curwp->w_markp = bp->b_markp;
				curwp->w_marko = bp->b_marko;
			} else {
				wp = wheadp;
				while (wp != NULL) {
					if (wp!=curwp && wp->w_bufp==bp) {
						curwp->w_dotp  = wp->w_dotp;
						curwp->w_doto  = wp->w_doto;
						curwp->w_markp = wp->w_markp;
						curwp->w_marko = wp->w_marko;
						break;
					}
					wp = wp->w_wndp;
				}
			}
			lp = curwp->w_dotp;
			i = curwp->w_ntrows/2;
			while (i-- && lback(lp)!=curbp->b_linep)
				lp = lback(lp);
			curwp->w_linep = lp;
			curwp->w_flag |= WFMODE|WFHARD;
			if (kbdmop == NULL)
				eprintf("[Old buffer]");
			return (TRUE);
		}
	}
	makename(bname, fname);
	while ((bp=bfind(bname, FALSE)) != NULL) {
		s = ereply("Buffer name: ", bname, NBUFN);
		if (s == ABORT)
			return (s);
		if (s == FALSE) {
			makename(bname, fname);
			break;
		}
	}
	if (bp==NULL && (bp=bfind(bname, TRUE))==NULL) {
		eprintf("Cannot create buffer");
		return (FALSE);
	}
	if (--curbp->b_nwnd == 0) {
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;
	curwp->w_bufp = bp;
	curbp->b_nwnd++;
	return (readin(fname));
}

static int
readin(char * fname)
{
	register LINE	*lp1;
	register LINE	*lp2;
	register int	i;
	register WINDOW	*wp;
	register BUFFER	*bp;
	register int	s;
	int		nbytes;
	register int	nline;
	char		*line;

	if(filldir(fname))return TRUE;
	{FILE*f=fopen(fname,"r");if(f){char b[512];size_t n=fread(b,1,sizeof b,f),k=0;fclose(f);
	int bin=n>=4&&!memcmp(b,"%PDF",4);
	while(!bin&&k<n){unsigned c=(unsigned char)b[k++];if(c<32&&c!=9&&c!=10&&c!=12&&c!=13)bin=1;}
	if(bin){if(!fork()){
#ifdef __APPLE__
	execlp("open","open",fname,(char*)0);
#else
	execlp("xdg-open","xdg-open",fname,(char*)0);
#endif
	_exit(0);}eprintf("[opened %s]",fname);return TRUE;}}}
	bp = curbp;
	if ((s=bclear(bp)) != TRUE)
		return (s);
	bp->b_flag &= ~BFCHG;
	strcpy(bp->b_fname, fname);
	fwatch(fname);
	if ((s=ffropen(fname)) == FIOERR)
		goto out;
	if (s == FIOFNF) {
		if (kbdmop == NULL)
			eprintf("[New file]");
		goto out;
	}
	nline = 0;
	while ((s=ffgetline(&line, &nbytes)) == FIOSUC) {
		if ((lp1=lalloc(nbytes)) == NULL) {
			s = FIOERR;
			break;
		}
		lp2 = lback(curbp->b_linep);
		lp2->l_fp = lp1;
		lp1->l_fp = curbp->b_linep;
		lp1->l_bp = lp2;
		curbp->b_linep->l_bp = lp1;
		for (i=0; i<nbytes; ++i)
			lputc(lp1, i, line[i]);
		++nline;
	}
	ffclose();
	if (s==FIOEOF && kbdmop==NULL) {
		if (nline == 1)
			eprintf("[Read 1 line]");
		else
			eprintf("[Read %d lines]", nline);
	}
out:
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == curbp) {
			wp->w_linep = lforw(curbp->b_linep);
			wp->w_dotp  = lforw(curbp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
			wp->w_flag |= WFMODE|WFHARD;
		}
	}
	if (s == FIOERR)
		return (FALSE);
	return (TRUE);
}

typedef struct{char n[64];char d;}Dent;
static int dentcmp(const void*a,const void*b){Dent*x=(Dent*)a,*y=(Dent*)b;if(x->d!=y->d)return y->d-x->d;return strcasecmp(x->n,y->n);}
static int
filldir(char *p)
{DIR*d;struct dirent*e;LINE*l;int n,c=0,i;char s[80];Dent ents[512];
if(!(d=opendir(p)))return 0;bclear(curbp);chdir(p);getcwd(curbp->b_fname,NFILEN);
while((e=readdir(d))&&c<512){if(e->d_name[0]=='.'&&!e->d_name[1])continue;ents[c].d=e->d_type==DT_DIR;strlcpy(ents[c++].n,e->d_name,64);}
closedir(d);qsort(ents,c,sizeof(Dent),dentcmp);for(i=0;i<c;i++){
n=sprintf(s,"%s%s",ents[i].d?"> ":"  ",ents[i].n);if((l=lalloc(n))){
l->l_bp=lback(curbp->b_linep);l->l_bp->l_fp=l;l->l_fp=curbp->b_linep;
curbp->b_linep->l_bp=l;while(n--)lputc(l,n,s[n]);}}dirmode=1;dirsl=0;dirsrch[0]=0;
curwp->w_linep=curwp->w_dotp=lforw(curbp->b_linep);curwp->w_doto=0;curwp->w_flag|=WFHARD;return 1;}
static int
backdir(int f, int n, int k)
{if(dirmode){filldir("..");}else{char d[80],*p;strcpy(d,curbp->b_fname);p=strrchr(d,'/');if(p)*p=0;else*d=0;filldir(*d?d:".");}return TRUE;}

static void
makename(char * bname, char * fname)
{
	register char	*cp1;
	register char	*cp2;

	cp1 = &fname[0];
	while (*cp1 != 0)
		++cp1;
#ifdef	BDC2
	while (cp1!=&fname[0] && cp1[-1]!=BDC1 && cp1[-1]!=BDC2)
		--cp1;
#else
	while (cp1!=&fname[0] && cp1[-1]!=BDC1)
		--cp1;
#endif
	cp2 = &bname[0];
#ifdef	BDC3
	while (cp2!=&bname[NBUFN-1] && *cp1!=0 && *cp1!=BDC3)
		*cp2++ = *cp1++;
#else
	while (cp2!=&bname[NBUFN-1] && *cp1!=0)
		*cp2++ = *cp1++;
#endif
	*cp2 = 0;
}

static int
filewrite(int f, int n, int k)
{
	register WINDOW	*wp;
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Write file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	if ((s=writeout(fname)) == TRUE) {
		strcpy(curbp->b_fname, fname);
		curbp->b_flag &= ~BFCHG;
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
	return (s);
}

static int
filesave(int f, int n, int k)
{
	register WINDOW	*wp;
	register int	s;

	if ((curbp->b_flag&BFCHG) == 0)
		return (TRUE);
	if (curbp->b_fname[0] == 0) {
		eprintf("No file name");
		return (FALSE);
	}
	if ((s=writeout(curbp->b_fname)) == TRUE) {
		curbp->b_flag &= ~BFCHG;
		wp = wheadp;
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
	return (s);
}

static int
writeout(char * fn)
{
	register int	s;
	register LINE	*lp;
	register int	nline;

	if ((s=ffwopen(fn)) != FIOSUC)
		return (FALSE);
	lp = lforw(curbp->b_linep);
	nline = 0;
	while (lp != curbp->b_linep) {
		if ((s=ffputline(&lp->l_text[0], llength(lp))) != FIOSUC)
			break;
		++nline;
		lp = lforw(lp);
	}
	if (s == FIOSUC) {
		s = ffclose();
		if (s==FIOSUC && kbdmop==NULL) {
			if (nline == 1)
				eprintf("[Wrote 1 line]");
			else
				eprintf("[Wrote %d lines]", nline);
		}
	} else
		ffclose();
	if (s != FIOSUC)
		return (FALSE);
	return (TRUE);
}

static int
filename(int f, int n, int k)
{
	register WINDOW	*wp;
	register int	s;
	char	 	fname[NFILEN];

	if ((s=ereply("Name: ", fname, NFILEN)) == ABORT)
		return (s);
	adjustcase(fname);
	strcpy(curbp->b_fname, fname);
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}
	return (TRUE);
}

static int
showcpos(int f, int n, int k)
{
	register LINE	*clp;
	register int	cbo;
	register int	nchar;
	register int	cchar;
	register int	nline;
	register int	cline;
	register int	cbyte;
	register int	ratio;
	register int	row;
	register int	col;
	register int	i;
	register int	c;

	clp = lforw(curbp->b_linep);
	cbo = 0;
	nchar = 0;
	nline = 1;
	for (;;) {
		if (clp == curwp->w_dotp) {
			cline = nline;
			if (cbo == curwp->w_doto) {
				cchar = nchar;
				if (cbo == llength(clp))
					cbyte = '\n';
				else
					cbyte = lgetc(clp, cbo);
			}
		}
		if (cbo == llength(clp)) {
			if (clp == curbp->b_linep)
				break;
			clp = lforw(clp);
			cbo = 0;
			++nline;
		} else
			++cbo;
		++nchar;
	}
	row = curwp->w_toprow;
	clp = curwp->w_linep;
	while (clp!=curbp->b_linep && clp!=curwp->w_dotp) {
		++row;
		clp = lforw(clp);
	}
	++row;
	col = 0;
	for (i=0; i<curwp->w_doto; ++i) {
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			col |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++col;
		++col;
	}
	++col;
	ratio = 0;
	if (nchar != 0) {
		ratio = (100L*cchar) / nchar;
		if (ratio==0 && cchar!=0)
			ratio = 1;
	}
	eprintf("[CH:0%o Line:%d Row:%d Col:%d %d%% of %d]",
		cbyte, cline, row, col, ratio, nchar);
	return (TRUE);
}

static int
twiddle(int f, int n, int k)
{
	register LINE	*dotp;
	register int	doto;
	register int	cl;
	register int	cr;

	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	if (doto==llength(dotp) && --doto<0)
		return (FALSE);
	cr = lgetc(dotp, doto);
	if (--doto < 0)
		return (FALSE);
	cl = lgetc(dotp, doto);
	lputc(dotp, doto+0, cr);
	lputc(dotp, doto+1, cl);
	lchange(WFEDIT);
	return (TRUE);
}

static int
quote(int f, int n, int k)
{
	register int	s;
	register int	c;

	if (kbdmop != NULL)
		c = *kbdmop++;
	else {
		c = ttgetc();
		if (kbdmip != NULL) {
			if (kbdmip > &kbdm[NKBDM-4]) {
				ctrlg(FALSE, 0, KRANDOM);
				return (ABORT);
			}
			*kbdmip++ = c;
		}
	}
	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return (s);
	}
	return (linsert(n, c));
}

static int
selfinsert(int f, int n, int k)
{
	register int	c;

	if (dirmode) {
		c = k & KCHAR;
		if (dirsl < 63) { LINE *lp; dirsrch[dirsl++] = c; dirsrch[dirsl] = 0;
			for (lp=lforw(curbp->b_linep); lp!=curbp->b_linep; lp=lforw(lp)) {
				char fn[80]; int i, nn=llength(lp); if(nn<3) continue;
				for(i=0;i<nn-2&&i<79;i++) fn[i]=lgetc(lp,i+2); fn[i]=0;
				if(strcasestr(fn,dirsrch)){curwp->w_dotp=lp;curwp->w_doto=0;curwp->w_flag|=WFMOVE;break;}
			}
		}
		eprintf("find: %s", dirsrch); return TRUE;
	}
	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	c = k & KCHAR;
	if ((k&KCTRL)!=0 && c>='@' && c<='_')
		c -= '@';
	return (linsert(n, c));
}

static int
openline(int f, int n, int k)
{
	register int	i;
	register int	s;

	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	i = n;
	do {
		s = lnewline();
	} while (s==TRUE && --i);
	if (s == TRUE)
		s = backchar(f, n, KRANDOM);
	return (s);
}

static int
newline(int f, int n, int k)
{
	register LINE	*lp;
	register int	s;
	if(dirmode){lp=curwp->w_dotp;char f_[80];int i_,n_=llength(lp);for(i_=0;i_<n_;i_++)f_[i_]=lgetc(lp,i_);f_[n_]=0;if(n_>1&&f_[0]=='>')filldir(f_+2);else{dirmode=0;readin(f_+2);}return TRUE;}
	if (n < 0)
		return (FALSE);
	while (n--) {
		lp = curwp->w_dotp;
		if (llength(lp) == curwp->w_doto
		&& lp != curbp->b_linep
		&& llength(lforw(lp)) == 0) {
			if ((s=forwchar(FALSE, 1, KRANDOM)) != TRUE)
				return (s);
		} else if ((s=lnewline()) != TRUE)
			return (s);
	}
	return (TRUE);
}

static int
deblank(int f, int n, int k)
{
	register LINE	*lp1;
	register LINE	*lp2;
	register int	nld;

	lp1 = curwp->w_dotp;
	while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	curwp->w_dotp = lforw(lp1);
	curwp->w_doto = 0;
	return (ldelete(nld, FALSE));
}

static int
indent(int f, int n, int k)
{
	register int	nicol;
	register int	c;
	register int	i;

	if (n < 0)
		return (FALSE);
	while (n--) {
		nicol = 0;
		for (i=0; i<llength(curwp->w_dotp); ++i) {
			c = lgetc(curwp->w_dotp, i);
			if (c!=' ' && c!='\t')
				break;
			if (c == '\t')
				nicol |= 0x07;
			++nicol;
		}
		if (lnewline() == FALSE
		|| ((i=nicol/8)!=0 && linsert(i, '\t')==FALSE)
		|| ((i=nicol%8)!=0 && linsert(i,  ' ')==FALSE))
			return (FALSE);
	}
	return (TRUE);
}

static int
forwdel(int f, int n, int k)
{
	if (n < 0)
		return (backdel(f, -n, KRANDOM));
	if (f != FALSE) {
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	return (ldelete(n, f));
}

static int
backdel(int f, int n, int k)
{
	register int	s;

	if (dirmode) { if(dirsl>0){dirsrch[--dirsl]=0;eprintf("find: %s",dirsl?dirsrch:"");} return TRUE; }
	if (n < 0)
		return (forwdel(f, -n, KRANDOM));
	if (f != FALSE) {
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	if ((s=backchar(f, n, KRANDOM)) == TRUE)
		s = ldelete(n, f);
	return (s);
}

static int
killline(int f, int n, int k)
{
	register int	chunk;
	register LINE	*nextp;

	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;
	if (f == FALSE) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto;
		if (chunk == 0)
			chunk = 1;
	} else if (n > 0) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
		nextp = lforw(curwp->w_dotp);
		while (--n) {
			if (nextp == curbp->b_linep)
				return (FALSE);
			chunk += llength(nextp)+1;
			nextp = lforw(nextp);
		}
	} else {
		chunk = curwp->w_doto;
		curwp->w_doto = 0;
		while (n++) {
			if (lback(curwp->w_dotp) == curbp->b_linep)
				break;
			curwp->w_dotp = lback(curwp->w_dotp);
			curwp->w_flag |= WFMOVE;
			chunk += llength(curwp->w_dotp)+1;
		}
	}
	return (ldelete(chunk, TRUE));
}

static int
yank(int f, int n, int k)
{
	register int	c;
	register int	i;
	register LINE	*lp;
	register int	nline;

	if (n < 0)
		return (FALSE);
	nline = 0;
	while (n--) {
		i = 0;
		while ((c=kremove(i)) >= 0) {
			if (c == '\n') {
				if (newline(FALSE, 1, KRANDOM) == FALSE)
					return (FALSE);
				++nline;
			} else {
				if (linsert(1, c) == FALSE)
					return (FALSE);
			}
			++i;
		}
	}
	lp = curwp->w_linep;
	if (curwp->w_dotp == lp) {
		while (nline-- && lback(lp)!=curbp->b_linep)
			lp = lback(lp);
		curwp->w_linep = lp;
		curwp->w_flag |= WFHARD;
	}
	return (TRUE);
}

static int
backword(int f, int n, int k)
{
	if (n < 0)
		return (forwword(f, -n, KRANDOM));
	if (backchar(FALSE, 1, KRANDOM) == FALSE)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (forwchar(FALSE, 1, KRANDOM));
}

static int
forwword(int f, int n, int k)
{
	if (n < 0)
		return (backword(f, -n, KRANDOM));
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

static int
upperword(int f, int n, int k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISLOWER(c) != FALSE) {
				c = TOUPPER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

static int
lowerword(int f, int n, int k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISUPPER(c) != FALSE) {
				c = TOLOWER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

static int
capword(int f, int n, int k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISLOWER(c) != FALSE) {
				c = TOUPPER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (ISUPPER(c) != FALSE) {
					c = TOLOWER(c);
					lputc(curwp->w_dotp, curwp->w_doto, c);
					lchange(WFHARD);
				}
				if (forwchar(FALSE, 1, KRANDOM) == FALSE)
					return (FALSE);
			}
		}
	}
	return (TRUE);
}

static int
delfword(int f, int n, int k)
{
	register int	size;
	register LINE	*dotp;
	register int	doto;

	if (n < 0)
		return (FALSE);
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;
			++size;
		}
		while (inword() != FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;
			++size;
		}
	}
out:
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return (ldelete(size, TRUE));
}

static int
delbword(int f, int n, int k)
{
	register int	size;

	if (n < 0)
		return (FALSE);
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;
	if (backchar(FALSE, 1, KRANDOM) == FALSE)
		return (TRUE);
	size = 1;
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;
			++size;
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;
			++size;
		}
	}
	if (forwchar(FALSE, 1, KRANDOM) == FALSE)
		return (FALSE);
	--size;
out:
	return (ldelete(size, TRUE));
}

static int
inword(void)
{
	if (curwp->w_doto == llength(curwp->w_dotp))
		return (FALSE);
	if (ISWORD(lgetc(curwp->w_dotp, curwp->w_doto)) != FALSE)
		return (TRUE);
	return (FALSE);
}

static int
killregion(int f, int n, int k)
{
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;
	curwp->w_dotp = region.r_linep;
	curwp->w_doto = region.r_offset;
	s = ldelete(region.r_size, TRUE);
	if (s == TRUE) clip_osc52();
	return (s);
}

static void
clip_osc52(void)
{
	int n = kused;
	FILE *fp;
	if (n <= 0 || n > 1000000) return;
#ifdef __APPLE__
	fp = popen("pbcopy", "w");
#else
	fp = popen("xclip -selection clipboard 2>/dev/null || xsel --clipboard --input 2>/dev/null || wl-copy 2>/dev/null", "w");
#endif
	if (fp) { fwrite(kbufp, 1, (size_t)n, fp); pclose(fp); }
}

static int
copyregion(int f, int n, int k)
{
	register LINE	*linep;
	register int	loffs;
	register int	s;
	REGION		region;

	{const char *mp = getenv("M_PID"); int pid = mp ? atoi(mp) : 0;
	 if (pid > 0) { kill(pid, SIGINT); eprintf("[Interrupt sent]"); return (TRUE); }}
	if ((s=getregion(&region)) != TRUE)
		return (s);
	if ((lastflag&CFKILL) == 0)
		kdelete();
	thisflag |= CFKILL;
	linep = region.r_linep;
	loffs = region.r_offset;
	while (region.r_size--) {
		if (loffs == llength(linep)) {
			if ((s=kinsert('\n')) != TRUE)
				return (s);
			linep = lforw(linep);
			loffs = 0;
		} else {
			if ((s=kinsert(lgetc(linep, loffs))) != TRUE)
				return (s);
			++loffs;
		}
	}
	clip_osc52();
	eprintf("[Copied %d bytes]", kused);
	return (TRUE);
}

static int
lowerregion(int f, int n, int k)
{
	register LINE	*linep;
	register int	loffs;
	register int	c;
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	lchange(WFHARD);
	linep = region.r_linep;
	loffs = region.r_offset;
	while (region.r_size--) {
		if (loffs == llength(linep)) {
			linep = lforw(linep);
			loffs = 0;
		} else {
			c = lgetc(linep, loffs);
			if (ISUPPER(c) != FALSE)
				lputc(linep, loffs, TOLOWER(c));
			++loffs;
		}
	}
	return (TRUE);
}

static int
upperregion(int f, int n, int k)
{
	register LINE	*linep;
	register int	loffs;
	register int	c;
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	lchange(WFHARD);
	linep = region.r_linep;
	loffs = region.r_offset;
	while (region.r_size--) {
		if (loffs == llength(linep)) {
			linep = lforw(linep);
			loffs = 0;
		} else {
			c = lgetc(linep, loffs);
			if (ISLOWER(c) != FALSE)
				lputc(linep, loffs, TOUPPER(c));
			++loffs;
		}
	}
	return (TRUE);
}

static int
getregion(REGION * rp)
{
	register LINE	*flp;
	register LINE	*blp;
	register long	fsize;
	register long	bsize;

	if (curwp->w_markp == NULL) {
		eprintf("No mark in this window");
		return (FALSE);
	}
	if (curwp->w_dotp == curwp->w_markp) {
		rp->r_linep = curwp->w_dotp;
		if (curwp->w_doto < curwp->w_marko) {
			rp->r_offset = curwp->w_doto;
			rp->r_size = curwp->w_marko-curwp->w_doto;
		} else {
			rp->r_offset = curwp->w_marko;
			rp->r_size = curwp->w_doto-curwp->w_marko;
		}
		return (TRUE);
	}
	blp = curwp->w_dotp;
	flp = curwp->w_dotp;
	bsize = curwp->w_doto;
	fsize = llength(flp)-curwp->w_doto+1;
	while (flp!=curbp->b_linep || lback(blp)!=curbp->b_linep) {
		if (flp != curbp->b_linep) {
			flp = lforw(flp);
			if (flp == curwp->w_markp) {
				rp->r_linep = curwp->w_dotp;
				rp->r_offset = curwp->w_doto;
				return (setsize(rp, fsize+curwp->w_marko));
			}
			fsize += llength(flp)+1;
		}
		if (lback(blp) != curbp->b_linep) {
			blp = lback(blp);
			bsize += llength(blp)+1;
			if (blp == curwp->w_markp) {
				rp->r_linep = blp;
				rp->r_offset = curwp->w_marko;
				return (setsize(rp, bsize-curwp->w_marko));
			}
		}
	}
	eprintf("Bug: lost mark");
	return (FALSE);
}

static int
setsize(REGION * rp, long size)
{
	if (size > 0x7FFFFFFF) {
		eprintf("Region is too large");
		return (FALSE);
	}
	rp->r_size = (int)size;
	return (TRUE);
}

static int
gotobol(int f, int n, int k)
{
	curwp->w_doto  = 0;
	return (TRUE);
}

static int
backchar(int f, int n, int k)
{
	register LINE	*lp;

	if (n < 0)
		return (forwchar(f, -n, KRANDOM));
	while (n--) {
		if (curwp->w_doto == 0) {
			if ((lp=lback(curwp->w_dotp)) == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lp;
			curwp->w_doto  = llength(lp);
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto--;
	}
	return (TRUE);
}

static int
gotoeol(int f, int n, int k)
{
	curwp->w_doto  = llength(curwp->w_dotp);
	return (TRUE);
}

static int
forwchar(int f, int n, int k)
{
	if (n < 0)
		return (backchar(f, -n, KRANDOM));
	while (n--) {
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (curwp->w_dotp == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lforw(curwp->w_dotp);
			curwp->w_doto  = 0;
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto++;
	}
	return (TRUE);
}

static int
gotobob(int f, int n, int k)
{
	curwp->w_dotp  = lforw(curbp->b_linep);
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

static int
gotoeob(int f, int n, int k)
{
	curwp->w_dotp  = curbp->b_linep;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

static int
forwline(int f, int n, int k)
{
	register LINE	*dlp;

	if (n < 0)
		return (backline(f, -n, KRANDOM));
	if ((lastflag&CFCPCN) == 0)
		setgoal();
	thisflag |= CFCPCN;
	dlp = curwp->w_dotp;
	while (n-- && dlp!=curbp->b_linep)
		dlp = lforw(dlp);
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

static int
backline(int f, int n, int k)
{
	register LINE	*dlp;

	if (n < 0)
		return (forwline(f, -n, KRANDOM));
	if ((lastflag&CFCPCN) == 0)
		setgoal();
	thisflag |= CFCPCN;
	dlp = curwp->w_dotp;
	while (n-- && lback(dlp)!=curbp->b_linep)
		dlp = lback(dlp);
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

static void
setgoal(void)
{
	register int	c;
	register int	i;

	curgoal = 0;
	for (i=0; i<curwp->w_doto; ++i) {
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			curgoal |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++curgoal;
		++curgoal;
	}
	if (curgoal >= ncol)
		curgoal = ncol-1;
}

static int
getgoal(LINE * dlp)
{
	register int	c;
	register int	col;
	register int	newcol;
	register int	dbo;

	col = 0;
	dbo = 0;
	while (dbo != llength(dlp)) {
		c = lgetc(dlp, dbo);
		newcol = col;
		if (c == '\t')
			newcol |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++newcol;
		++newcol;
		if (newcol > curgoal)
			break;
		col = newcol;
		++dbo;
	}
	return (dbo);
}

static int
forwpage(int f, int n, int k)
{
	register LINE	*lp;

	if (f == FALSE) {
		n = curwp->w_ntrows - 2;
		if (n <= 0)
			n = 1;
	} else if (n < 0)
		return (backpage(f, -n, KRANDOM));
#if	CVMVAS
	else
		n *= curwp->w_ntrows;
#endif
	lp = curwp->w_linep;
	while (n-- && lp!=curbp->b_linep)
		lp = lforw(lp);
	curwp->w_linep = lp;
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

static int
backpage(int f, int n, int k)
{
	register LINE	*lp;

	if (f == FALSE) {
		n = curwp->w_ntrows - 2;
		if (n <= 0)
			n = 1;
	} else if (n < 0)
		return (forwpage(f, -n, KRANDOM));
#if	CVMVAS
	else
		n *= curwp->w_ntrows;
#endif
	lp = curwp->w_linep;
	while (n-- && lback(lp)!=curbp->b_linep)
		lp = lback(lp);
	curwp->w_linep = lp;
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

static int
selectall(int f, int n, int k)
{
	curwp->w_dotp = lforw(curbp->b_linep);
	curwp->w_doto = 0;
	curwp->w_markp = lback(curbp->b_linep);
	curwp->w_marko = llength(lback(curbp->b_linep));
	curwp->w_flag |= WFHARD;
	eprintf("[All selected]");
	return (TRUE);
}

static int
setmark(int f, int n, int k)
{
	curwp->w_markp = curwp->w_dotp;
	curwp->w_marko = curwp->w_doto;
	if (kbdmop == NULL)
		eprintf("[Mark set]");
	return (TRUE);
}

static int
swapmark(int f, int n, int k)
{
	register LINE	*odotp;
	register int	odoto;

	if (curwp->w_markp == NULL) {
		eprintf("No mark in this window");
		return (FALSE);
	}
	odotp = curwp->w_dotp;
	odoto = curwp->w_doto;
	curwp->w_dotp  = curwp->w_markp;
	curwp->w_doto  = curwp->w_marko;
	curwp->w_markp = odotp;
	curwp->w_marko = odoto;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

static int
gotoline(int f, int n, int k)
{
	register LINE	*clp;
	register int	s;
	char		buf[32];

	if (f == FALSE) {
		if ((s=ereply("Goto line: ", buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n <= 0) {
		eprintf("Bad line");
		return (FALSE);
	}
	clp = lforw(curbp->b_linep);
	while (n != 1) {
		if (clp == curbp->b_linep) {
			eprintf("Line number too large");
			return (FALSE);
		}
		clp = lforw(clp);
		--n;
	}
	curwp->w_dotp = clp;
	curwp->w_doto = 0;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}
#define CCHR(x)		((x)-'@')

#define SRCH_BEGIN	(0)
#define	SRCH_FORW	(-1)
#define SRCH_BACK	(-2)
#define SRCH_PREV	(-3)
#define SRCH_NEXT	(-4)
#define SRCH_NOPR	(-5)
#define SRCH_ACCM	(-6)

typedef struct  {
	int	s_code;
	LINE	*s_dotp;
	int	s_doto;
}	SRCHCOM;

static	SRCHCOM	cmds[NSRCH];
static	int	cip;

static int	srch_lastdir = SRCH_NOPR;

static int
forwsearch(int f, int n, int k)
{
	register int	s;

	if ((s=readpattern("Search")) != TRUE)
		return (s);
	if (forwsrch() == FALSE) {
		eprintf("Not found");
		return (FALSE);
	}
	srch_lastdir = SRCH_FORW;
	return (TRUE);
}

static int
backsearch(int f, int n, int k)
{
	register int	s;

	if ((s=readpattern("Reverse search")) != TRUE)
		return (s);
	if (backsrch() == FALSE) {
		eprintf("Not found");
		return (FALSE);
	}
	srch_lastdir = SRCH_BACK;
	return (TRUE);
}

static int
searchagain(int f, int n, int k)
{
	if (srch_lastdir == SRCH_FORW) {
		if (forwsrch() == FALSE) {
			eprintf("Not found");
			return (FALSE);
		}
		return (TRUE);
	}
	if (srch_lastdir == SRCH_BACK) {
		if (backsrch() == FALSE) {
			eprintf("Not found");
			return (FALSE);
		}
		return (TRUE);
	}
	eprintf("No last search");
	return (FALSE);
}

static int
forwisearch(int f, int n, int k)
{
	return (isearch(SRCH_FORW));
}

static int
backisearch(int f, int n, int k)
{
	return (isearch(SRCH_BACK));
}

static int
isearch(int dir)
{
	register int	c;
	register LINE	*clp;
	register int	cbo;
	register int	success;
	int		pptr;

	for (cip=0; cip<NSRCH; cip++)
		cmds[cip].s_code = SRCH_NOPR;
	cip = 0;
	pptr = -1;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	is_lpush();
	is_cpush(SRCH_BEGIN);
	success = TRUE;
	is_prompt(dir, TRUE, success);
	for (;;) {
		update();
		switch (c = ttgetc()) {
		case CCHR('M'):
		case METACH:
			srch_lastdir = dir;
			eprintf("[Done]");
			return (TRUE);

		case CCHR('G'):
			curwp->w_dotp = clp;
			curwp->w_doto = cbo;
			curwp->w_flag |= WFMOVE;
			srch_lastdir = dir;
			ctrlg(FALSE, 0, KRANDOM);
			return (FALSE);

		case CCHR('S'):
		case CCHR('F'):
			if (dir == SRCH_BACK) {
				dir = SRCH_FORW;
				is_lpush();
				is_cpush(SRCH_FORW);
				success = TRUE;
			}

		case CCHR('N'):
			if (success==FALSE && dir==SRCH_FORW)
				break;
			is_lpush();
			forwchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_NEXT) != FALSE) {
				is_cpush(SRCH_NEXT);
				pptr = strlen(pat);
			} else {
				backchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir, FALSE, success);
			break;

		case CCHR('R'):
		case CCHR('B'):
			if (dir == SRCH_FORW) {
				dir = SRCH_BACK;
				is_lpush();
				is_cpush(SRCH_BACK);
				success = TRUE;
			}

		case CCHR('P'):
			if (success==FALSE && dir==SRCH_BACK)
				break;
			is_lpush();
			backchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_PREV) != FALSE) {
				is_cpush(SRCH_PREV);
				pptr = strlen(pat);
			} else {
				forwchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir,FALSE,success);
			break;

		case 0x7F:
			if (is_undo(&pptr, &dir) != TRUE)
				return (ABORT);
			if (is_peek() != SRCH_ACCM)
				success = TRUE;
			is_prompt(dir, FALSE, success);
			break;

		case CCHR('^'):
		case CCHR('Q'):
			c = ttgetc();
		case CCHR('U'):
		case CCHR('X'):
		case CCHR('J'):
			goto  addchar;

		default:
			if (ISCTRL(c) != FALSE) {
				c += '@';
				c |= KCTRL;
				success = execute(c, FALSE, 1);
				curwp->w_flag |= WFMOVE;
				return (success);
			}
		addchar:
			if (pptr == -1)
				pptr = 0;
			if (pptr == 0)
				success = TRUE;
			pat[pptr++] = c;
			if (pptr == NPAT) {
				eprintf("Pattern too long");
				ctrlg(FALSE, 0, KRANDOM);
				return (ABORT);
			}
			pat[pptr] = '\0';
			is_lpush();
			if (success != FALSE) {
				if (is_find(dir) != FALSE)
					is_cpush(c);
				else {
					success = FALSE;
					ttbeep();
					is_cpush(SRCH_ACCM);
				}
			} else
				is_cpush(SRCH_ACCM);
			is_prompt(dir, FALSE, success);
		}
	}
}

static void
is_cpush(int cmd)
{
	if (++cip >= NSRCH)
		cip = 0;
	cmds[cip].s_code = cmd;
}

static void
is_lpush(void)
{
	register int	ctp;

	ctp = cip+1;
	if (ctp >= NSRCH)
		ctp = 0;
	cmds[ctp].s_code = SRCH_NOPR;
	cmds[ctp].s_doto = curwp->w_doto;
	cmds[ctp].s_dotp = curwp->w_dotp;
}

static void
is_pop(void)
{
	if (cmds[cip].s_code != SRCH_NOPR) {
		curwp->w_doto  = cmds[cip].s_doto;
		curwp->w_dotp  = cmds[cip].s_dotp;
		curwp->w_flag |= WFMOVE;
		cmds[cip].s_code = SRCH_NOPR;
	}
	if (--cip <= 0)
		cip = NSRCH-1;
}

static int
is_peek(void)
{
	if (cip == 0)
		return (cmds[NSRCH-1].s_code);
	else
		return (cmds[cip-1].s_code);
}

static int
is_undo(int * pptr, int * dir)
{
	switch (cmds[cip].s_code) {
	case SRCH_NOPR:
	case SRCH_BEGIN:
	case SRCH_NEXT:
	case SRCH_PREV:
		break;

	case SRCH_FORW:
		*dir = SRCH_BACK;
		break;

	case SRCH_BACK:
		*dir = SRCH_FORW;
		break;

	case SRCH_ACCM:
	default:
		*pptr -= 1;
		if (*pptr < 0)
			*pptr = 0;
		pat[*pptr] = '\0';
		break;
	}
	is_pop();
	return (TRUE);
}

static int
is_find(int dir)
{
	register int	plen;

	plen = strlen(pat);
	if (plen != 0) {
		if (dir==SRCH_FORW || dir==SRCH_NEXT) {
			backchar(FALSE, plen, KRANDOM);
			if (forwsrch() == FALSE) {
				forwchar(FALSE, plen, KRANDOM);
				return (FALSE);
			}
			return (TRUE);
		}
		if (dir==SRCH_BACK || dir==SRCH_PREV) {
			forwchar(FALSE, plen, KRANDOM);
			if (backsrch() == FALSE) {
				backchar(FALSE, plen, KRANDOM);
				return (FALSE);
			}
			return (TRUE);
		}
		eprintf("bad call to is_find");
		ctrlg(FALSE, 0, KRANDOM);
		return (FALSE);
	}
	return (FALSE);
}

static void
is_prompt(int dir, int flag, int success)
{
	if (dir == SRCH_FORW) {
		if (success != FALSE)
			is_dspl("i-search forward", flag);
		else
			is_dspl("failing i-search forward", flag);
	} else if (dir == SRCH_BACK) {
		if (success != FALSE)
			is_dspl("i-search backward", flag);
		else
			is_dspl("failing i-search backward", flag);
	}
}

static void
is_dspl(char * prompt, int flag)
{
	if (flag != FALSE)
		eprintf("%s [%s]", prompt, pat);
	else
		eprintf("%s: %s", prompt, pat);
}

static int
queryrepl(int f, int n, int k)
{
	register int	s;
	char		news[NPAT];
	register int	kludge;
	LINE		*clp;
	int		cbo;
	int		rcnt = 0;
	int		plen;

	if ((s=readpattern("Old string")) != TRUE)
		return (s);
	if ((s=ereply("New string: ",news, NPAT)) == ABORT)
		return (s);
	if (s == FALSE)
		news[0] = '\0';
	eprintf("Query Replace:  [%s] -> [%s]", pat, news);
	plen = strlen(pat);

	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	while (forwsrch() == TRUE) {
	retry:
		update();
		switch (ttgetc()) {
		case ' ':
		case ',':
			kludge = (curwp->w_dotp == clp);
			if (lreplace(plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			if (kludge != FALSE)
				clp = curwp->w_dotp;
			break;

		case '.':
			kludge = (curwp->w_dotp == clp);
			if (lreplace(plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			if (kludge != FALSE)
				clp = curwp->w_dotp;
			goto stopsearch;

		case CCHR('G'):
			ctrlg(FALSE, 0, KRANDOM);
			goto stopsearch;

		case '!':
			do {
				kludge = (curwp->w_dotp == clp);
				if (lreplace(plen, news, f) == FALSE)
					return (FALSE);
				rcnt++;
				if (kludge != FALSE)
					clp = curwp->w_dotp;
			} while (forwsrch() == TRUE);
			goto stopsearch;

		case 'n':
			break;

		default:
eprintf("<SP>[,] replace, [.] rep-end, [n] don't, [!] repl rest [C-G] quit");
			goto retry;
		}
	}
stopsearch:
	curwp->w_dotp = clp;
	curwp->w_doto = cbo;
	curwp->w_flag |= WFHARD;
	update();
	if (rcnt == 0)
		eprintf("[No replacements done]");
	else if (rcnt == 1)
		eprintf("[1 replacement done]");
	else
		eprintf("[%d replacements done]", rcnt);
	return (TRUE);
}

static int
forwsrch(void)
{
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register char	*pp;
	register int	c;

	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	while (clp != curbp->b_linep) {
		if (cbo == llength(clp)) {
			clp = lforw(clp);
			cbo = 0;
			c = '\n';
		} else
			c = lgetc(clp, cbo++);
		if (eq(c, pat[0]) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = &pat[1];
			while (*pp != 0) {
				if (tlp == curbp->b_linep)
					goto fail;
				if (tbo == llength(tlp)) {
					tlp = lforw(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = 0;
					c = '\n';
				} else
					c = lgetc(tlp, tbo++);
				if (eq(c, *pp++) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
	return (FALSE);
}

static int
backsrch(void)
{
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register int	c;
	register char	*epp;
	register char	*pp;

	for (epp = &pat[0]; epp[1] != 0; ++epp)
		;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	for (;;) {
		if (cbo == 0) {
			clp = lback(clp);
			if (clp == curbp->b_linep)
				return (FALSE);
			cbo = llength(clp)+1;
		}
		if (--cbo == llength(clp))
			c = '\n';
		else
			c = lgetc(clp,cbo);
		if (eq(c, *epp) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = epp;
			while (pp != &pat[0]) {
				if (tbo == 0) {
					tlp = lback(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = llength(tlp)+1;
				}
				if (--tbo == llength(tlp))
					c = '\n';
				else
					c = lgetc(tlp,tbo);
				if (eq(c, *--pp) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
}

static int
eq(int bc, int pc)
{
	register int	ibc;
	register int	ipc;

	ibc = bc & 0xFF;
	ipc = pc & 0xFF;
	if (ISLOWER(ibc) != FALSE)
		ibc = TOUPPER(ibc);
	if (ISLOWER(ipc) != FALSE)
		ipc = TOUPPER(ipc);
	if (ibc == ipc)
		return (TRUE);
	return (FALSE);
}

static int
readpattern(char * prompt)
{
	register int	s;
	char		tpat[NPAT];

	s = ereply("%s [%s]: ", tpat, NPAT, prompt, pat);
	if (s == TRUE)
		strcpy(pat, tpat);
	else if (s==FALSE && pat[0]!=0)
		s = TRUE;
	return (s);
}

static int
getkey(void)
{
	register int	c;

	c = getkbd();
	if (c == METACH)
		c = KMETA | getctl();
	else if (c == CTRLCH)
		c = KCTRL | getctl();
	else if (c == CTMECH)
		c = KCTRL | KMETA | getctl();
	else if (c>=0x00 && c<=0x1F)
		c = KCTRL | (c+'@');
	if (c == (KCTRL|'X'))
		c = KCTLX | getctl();
	return (c);
}

static int
getctl(void)
{
	register int	c;

	c = ttgetc();
	if (ISLOWER(c) != FALSE)
		c = TOUPPER(c);
	if (c>=0x00 && c<=0x1F)
		c = KCTRL | (c+'@');
	return (c);
}

static void
keyname(char * cp, int k)
{
	register char	*np;
	char		nbuf[3];

	static	char	hex[] = {
		'0',	'1',	'2',	'3',
		'4',	'5',	'6',	'7',
		'8',	'9',	'A',	'B',
		'C',	'D',	'E',	'F'
	};

	if ((k&KCTLX) != 0) {
		*cp++ = 'C';
		*cp++ = '-';
		*cp++ = 'X';
		*cp++ = ' ';
		k &= ~KCTLX;
	}
	if ((k&KCHAR)>=KFIRST && (k&KCHAR)<=KLAST) {
		if ((np=keystrings[(k&KCHAR)-KFIRST]) != NULL) {
			if ((k&KCTRL) != 0) {
				*cp++ = 'C';
				*cp++ = '-';
			}
			if ((k&KMETA) != 0) {
				*cp++ = 'M';
				*cp++ = '-';
			}
			strcpy(cp, np);
			return;
		}
	}
	if ((k&~KMETA) == (KCTRL|'I'))
		np = "Tab";
	else if ((k&~KMETA) == (KCTRL|'M'))
		np = "Return";
	else if ((k&~KMETA) == (KCTRL|'H'))
		np = "Backspace";
	else if ((k&~KMETA) == ' ')
		np = "Space";
	else if ((k&~KMETA) == 0x7F)
		np = "Rubout";
	else {
		if ((k&KCTRL) != 0) {
			*cp++ = 'C';
			*cp++ = '-';
		}
		np = &nbuf[0];
		if (((k&KCHAR)>=0x20 && (k&KCHAR)<=0x7E)
		||  ((k&KCHAR)>=0xA0 && (k&KCHAR)<=0xFE)) {
			nbuf[0] = k&KCHAR;
			nbuf[1] = 0;
		} else {
			nbuf[0] = hex[(k>>4)&0x0F];
			nbuf[1] = hex[k&0x0F];
			nbuf[2] = 0;
		}
	}
	if ((k&KMETA) != 0) {
		*cp++ = 'M';
		*cp++ = '-';
	}
	strcpy(cp, np);
}

static int
bindtokey(int f, int n, int k)
{
	register int	s;
	register SYMBOL	*sp;
	register int	c;
	char		xname[NXNAME];

	if (kbdmip!=NULL || kbdmop!=NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	if ((s=ereadf("Function: ", xname, NXNAME, EFAUTO)) != TRUE)
		return (s);
	if ((sp=symlookup(xname)) == NULL) {
		eprintf("Unknown function for binding");
		return (FALSE);
	}
	eputc(' ');
	eputc('K');
	eputc('e');
	eputc('y');
	eputc(':');
	eputc(' ');
	ttflush();
	c = getkey();
	keyname(xname, c);
	eputs(xname);
	ttflush();
	if (binding[c] != NULL)
		--binding[c]->s_nkey;
	binding[c] = sp;
	++sp->s_nkey;
	return (TRUE);
}

static int
extend(int f, int n, int k)
{
	register SYMBOL	*sp;
	register int	s;
	char		xname[NXNAME];

	if ((s=ereadf(": ", xname, NXNAME, EFNEW|EFAUTO)) != TRUE)
		return (s);
	if ((sp=symlookup(xname)) != NULL)
		return ((*sp->s_funcp)(f, n, KRANDOM));
	eprintf("Unknown extended command");
	return (ABORT);
}

static int
help(int f, int n, int k)
{
	register SYMBOL	*sp;
	register int	c;
	char		b[20];

	c = getkey();
	keyname(b, c);
	if ((sp=binding[c]) == NULL)
		eprintf("[%s is unbound]", b);
	else
		eprintf("[%s is bound to %s]", b, sp->s_name);
	return (TRUE);
}

static int
wallchart(int f, int n, int k)
{
	register int	s;
	register int	key;
	register SYMBOL	*sp;
	register char	*cp1;
	register char	*cp2;
	char		buf[64];

	if ((s=bclear(blistp)) != TRUE)
		return (s);
	(void) strcpy(blistp->b_fname, "");
	for (key=0; key<NKEYS; ++key) {
		sp = binding[key];
		if (sp != NULL
		&& (f!=FALSE || strcmp(sp->s_name, "ins-self")!=0)) {
			keyname(buf, key);
			cp1 = &buf[0];
			while (*cp1 != 0)
				++cp1;
			while (cp1 < &buf[16])
				*cp1++ = ' ';
			cp2 = sp->s_name;
			while ((*cp1++ = *cp2++))
				;
			if (addline(buf) == FALSE)
				return (FALSE);
		}
	}
	return (popblist());
}

#define	XCHAR	int
#define	XSHORT	int

typedef	struct	{
	short	v_hash;
	short	v_flag;
	short	v_color;
	XSHORT	v_cost;
	char	v_text[NCOL];
	char	v_attr[NCOL];
}	VIDEO;

#define HL_NORM 0
#define HL_KW   1
#define HL_STR  2
#define HL_CMT  3
#define HL_NUM  4
#define HL_PRE  5
#define HL_SEL  6

static const char *hl_colors[] = {
	"\033[m",       /* HL_NORM */
	"\033[33m",     /* HL_KW  yellow */
	"\033[32m",     /* HL_STR green */
	"\033[36m",     /* HL_CMT cyan */
	"\033[35m",     /* HL_NUM magenta */
	"\033[31m",     /* HL_PRE red/preprocessor */
	"\033[7m",      /* HL_SEL reverse video */
};

static const char *c_kw[] = {
	"auto","break","case","char","const","continue","default","do",
	"double","else","enum","extern","float","for","goto","if","int",
	"long","register","return","short","signed","sizeof","static",
	"struct","switch","typedef","union","unsigned","void","volatile",
	"while","NULL","TRUE","FALSE","define","include","ifdef","ifndef",
	"endif","elif","undef","pragma",NULL
};

static int is_ckw(const char *s, int len) {
	const char **k;
	for (k = c_kw; *k; k++)
		if ((int)strlen(*k) == len && !memcmp(s, *k, (size_t)len))
			return 1;
	return 0;
}

static int
off2col(LINE *lp, int off)
{
	int c = 0, k;
	for (k = 0; k < off && k < llength(lp); k++) {
		if (lgetc(lp,k)=='\t') c|=7;
		c++;
	}
	return c - hoff;
}

static void
hl_sel(VIDEO *vp, LINE *lp, int cols, WINDOW *wp)
{
	LINE *mp, *dp, *s1, *s2, *p;
	int mo, doto, o1, o2, df, c1, c2, ci, in;
	mp = wp->w_markp; dp = wp->w_dotp;
	mo = wp->w_marko; doto = wp->w_doto;
	if (!mp) return;
	if (dp == mp) {
		if (doto == mo) return;
		s1 = s2 = dp;
		o1 = doto < mo ? doto : mo;
		o2 = doto < mo ? mo : doto;
	} else {
		df = 0;
		for (p = lforw(dp); p != wp->w_bufp->b_linep; p = lforw(p))
			if (p == mp) { df = 1; break; }
		if (df) { s1 = dp; o1 = doto; s2 = mp; o2 = mo; }
		else    { s1 = mp; o1 = mo; s2 = dp; o2 = doto; }
	}
	if (lp == s1 && lp == s2) {
		c1 = off2col(lp, o1); c2 = off2col(lp, o2);
		for (ci = c1 < 0 ? 0 : c1; ci < c2 && ci < cols; ci++)
			vp->v_attr[ci] = HL_SEL;
		return;
	}
	/* check if lp is in [s1..s2] */
	in = 0;
	if (lp == s1 || lp == s2) in = 1;
	else { for (p = lforw(s1); p != wp->w_bufp->b_linep && p != s2; p = lforw(p))
		if (p == lp) { in = 1; break; } }
	if (!in) return;
	if (lp == s1) {
		c1 = off2col(lp, o1);
		for (ci = c1 < 0 ? 0 : c1; ci < cols; ci++) vp->v_attr[ci] = HL_SEL;
	} else if (lp == s2) {
		c2 = off2col(lp, o2);
		for (ci = 0; ci < c2 && ci < cols; ci++) vp->v_attr[ci] = HL_SEL;
	} else {
		for (ci = 0; ci < cols; ci++) vp->v_attr[ci] = HL_SEL;
	}
}

static void
hl_line(VIDEO *vp, int cols)
{
	int i, st = HL_NORM;
	char *t = vp->v_text;
	char *a = vp->v_attr;

	for (i = 0; i < cols; i++) {
		int c = t[i] & 0xFF;
		if (st == HL_CMT) { a[i] = HL_CMT; continue; }
		if (st == HL_STR) {
			a[i] = HL_STR;
			if (c == '"' && i > 0 && t[i-1] != '\\') st = HL_NORM;
			continue;
		}
		if (c == '/' && i+1 < cols && t[i+1] == '/') {
			st = HL_CMT; a[i] = HL_CMT; continue;
		}
		if (c == '/' && i+1 < cols && t[i+1] == '*') {
			st = HL_CMT; a[i] = HL_CMT; continue;
		}
		if (c == '"') { st = HL_STR; a[i] = HL_STR; continue; }
		if (c == '#' && st == HL_NORM) {
			int j = i + 1;
			while (j < cols && t[j] == ' ') j++;
			int ws = j;
			while (j < cols && ((t[j]>='a'&&t[j]<='z')||(t[j]>='A'&&t[j]<='Z'))) j++;
			if (j > ws && is_ckw(t+ws, j-ws)) {
				for (int k = i; k < j; k++) a[k] = HL_PRE;
				i = j - 1; continue;
			}
		}
		if ((c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_') {
			int s = i;
			while (i < cols && ((t[i]>='a'&&t[i]<='z')||(t[i]>='A'&&t[i]<='Z')||
				(t[i]>='0'&&t[i]<='9')||t[i]=='_')) i++;
			int isk = is_ckw(t+s, i-s);
			for (int j = s; j < i; j++) a[j] = isk ? HL_KW : HL_NORM;
			i--; continue;
		}
		if (c>='0' && c<='9') {
			a[i] = HL_NUM; continue;
		}
		a[i] = HL_NORM;
	}
}

static void	hash(VIDEO *);
static void	uline(int, VIDEO *, VIDEO *);
static void	ucopy(VIDEO *, VIDEO *);
#define	VFCHG	0x0001
#define	VFHBAD	0x0002

typedef	struct	{
	XCHAR	s_itrace;
	XCHAR	s_jtrace;
	XSHORT	s_cost;
}	SCORE;

int	sgarbf	= TRUE;
int	vtrow	= 0;
int	vtcol	= 0;
int	tthue	= CNONE;
int	ttrow	= HUGE;
int	ttcol	= HUGE;
int	tttop	= HUGE;
int	ttbot	= HUGE;

static VIDEO	*vscreen[NROW-1];
static VIDEO	*pscreen[NROW-1];
static VIDEO	video[2*(NROW-1)];
static VIDEO	blanks;

#if	GOSLING

static SCORE	score[NROW*NROW];
#endif

static void
vtinit(void)
{
	register VIDEO	*vp;
	register int	i;

	ttopen();
	ttinit();
	vp = &video[0];
	for (i=0; i<NROW-1; ++i) {
		vscreen[i] = vp;
		++vp;
		pscreen[i] = vp;
		++vp;
	}
	blanks.v_color = CTEXT;
	for (i=0; i<NCOL; ++i)
		blanks.v_text[i] = ' ';
}

static void
vttidy(void)
{
	ttcolor(CTEXT);
	ttnowindow();
	ttmove(nrow-1, 0);
	tteeol();
	tttidy();
	ttflush();
	ttclose();
}

static void
vtmove(int row, int col)
{
	vtrow = row;
	vtcol = col;
}

static void
vtputc(int c)
{
	register VIDEO	*vp;

	vp = vscreen[vtrow];
	if(vtcol<0){vtcol++;return;}
	if (vtcol >= ncol) return;
	if (c == '\t') {
		do {
			vtputc(' ');
		} while (vtcol<ncol && (vtcol&0x07)!=0);
	} else if (ISCTRL(c) != FALSE) {
		vtputc('^');
		vtputc(c ^ 0x40);
	} else
		vp->v_text[vtcol++] = c;
}

static void
vteeol(void)
{
	register VIDEO	*vp;

	vp = vscreen[vtrow];
	while (vtcol < ncol)
		vp->v_text[vtcol++] = ' ';
	if(vtrow==0){const char*sp="[SPEAK]";const char*st="[STOP]";const char*bt="[ADD FILE]";const char*xx="[X]";int bi;int active=speak_running();
		/* pre-clear 31-col strip; [STOP] painted only while `a say` is alive */
		for(bi=ncol-31;bi<ncol;bi++)vp->v_text[bi]=' ';
		for(bi=0;sp[bi];bi++){vp->v_text[ncol-31+bi]=sp[bi];vp->v_attr[ncol-31+bi]=HL_NUM;}
		if(active)for(bi=0;st[bi];bi++){vp->v_text[ncol-23+bi]=st[bi];vp->v_attr[ncol-23+bi]=HL_KW;}
		for(bi=0;bt[bi];bi++){vp->v_text[ncol-15+bi]=bt[bi];vp->v_attr[ncol-15+bi]=HL_STR;}
		for(bi=0;xx[bi];bi++){vp->v_text[ncol-3+bi]=xx[bi];vp->v_attr[ncol-3+bi]=HL_KW;}}
	else if(!nosb&&vtrow>=curwp->w_toprow&&vtrow<curwp->w_toprow+curwp->w_ntrows){
		if(vtrow>=sb_top&&vtrow<=sb_bot){vp->v_text[ncol-2]='|';vp->v_text[ncol-1]='|';}}
}

static int wrap_rows(LINE *lp) {
	int j, col = 0, rows = 1;
	for (j = 0; j < llength(lp); j++) {
		int c = lgetc(lp, j) & 0xFF;
		int w = c=='\t' ? (8-(col&7)) : ISCTRL(c) ? 2 : 1;
		if (col+w>ncol-2 && col>0) { rows++; col = 0; }
		col += w;
	}
	return rows;
}

static int wrap_render(LINE *lp, int row, int max_row, WINDOW *wp, int skip) {
	int j = 0, c, col = 0;
	if (row >= max_row) return row;
	if (skip > 0) { int vr = 0;
		for (; j < llength(lp); j++) {
			c = lgetc(lp, j);
			int w = c=='\t' ? (8-(col&7)) : ISCTRL(c) ? 2 : 1;
			if (col+w>ncol-2 && col>0) { vr++; col = 0; if (vr == skip) break; }
			col += w;
		}
		col = 0;
	}
	vscreen[row]->v_color = CTEXT;
	vscreen[row]->v_flag |= (VFCHG|VFHBAD);
	vtmove(row, 0);
	for (; j < llength(lp); j++) {
		c = lgetc(lp, j);
		int w = c=='\t' ? (8-(col&7)) : ISCTRL(c) ? 2 : 1;
		if (col+w>ncol-2 && col>0) {
			vteeol();
			hl_line(vscreen[row], ncol);
			hl_sel(vscreen[row], lp, ncol, wp);
			if(row==0){int bi;for(bi=0;bi<7;bi++)vscreen[0]->v_attr[ncol-31+bi]=HL_NUM;for(bi=0;bi<6;bi++)vscreen[0]->v_attr[ncol-23+bi]=HL_KW;for(bi=0;bi<10;bi++)vscreen[0]->v_attr[ncol-15+bi]=HL_STR;for(bi=0;bi<3;bi++)vscreen[0]->v_attr[ncol-3+bi]=HL_KW;}
			row++;
			if (row >= max_row) return row;
			vscreen[row]->v_color = CTEXT;
			vscreen[row]->v_flag |= (VFCHG|VFHBAD);
			vtmove(row, 0);
			col = 0;
		}
		vtputc(c);
		col += w;
	}
	vteeol();
	hl_line(vscreen[row], ncol);
	hl_sel(vscreen[row], lp, ncol, wp);
	if(row==0){int bi;for(bi=0;bi<7;bi++)vscreen[0]->v_attr[ncol-31+bi]=HL_NUM;for(bi=0;bi<6;bi++)vscreen[0]->v_attr[ncol-23+bi]=HL_KW;for(bi=0;bi<10;bi++)vscreen[0]->v_attr[ncol-15+bi]=HL_STR;for(bi=0;bi<3;bi++)vscreen[0]->v_attr[ncol-3+bi]=HL_KW;}
	return row + 1;
}

static void
update(void)
{
	register LINE	*lp;
	register WINDOW	*wp;
	register VIDEO	*vp1;
	register VIDEO	*vp2;
	register int	i;
	register int	c;
	register int	hflag;
	register int	currow;
	register int	curcol;
	register int	offs;
	register int	size;

	if (curmsgf!=FALSE || newmsgf!=FALSE) {
		wp = wheadp;
		while (wp != NULL) {
			wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
	curmsgf = newmsgf;
	hflag = FALSE;
	if (curwp->w_markp && (curwp->w_flag & WFMOVE))
		curwp->w_flag |= WFHARD;
	{int t=0,a=0,h;LINE*p;for(p=lforw(curbp->b_linep);p!=curbp->b_linep;p=lforw(p)){if(p==curwp->w_linep)a=t;t++;}
	h=curwp->w_ntrows;if(t<h)t=h;sb_top=curwp->w_toprow+a*h/t;sb_bot=sb_top+h/6;if(sb_bot<sb_top+4)sb_bot=sb_top+4;if(sb_bot>sb_top+h-1)sb_bot=sb_top+h-1;}
	hoff=0;
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag != 0) {
			if ((wp->w_flag&WFFORCE) == 0) {
				lp = wp->w_linep; i = 0;
				while (i < wp->w_ntrows) {
					if (lp == wp->w_dotp) goto out;
					if (lp == wp->w_bufp->b_linep) break;
					i += wrap_rows(lp);
					lp = lforw(lp);
				}
			}
			i = wp->w_force;
			if (i > 0) {
				--i;
				if (i >= wp->w_ntrows)
					i = wp->w_ntrows-1;
			} else if (i < 0) {
				i += wp->w_ntrows;
				if (i < 0)
					i = 0;
			} else
				i = wp->w_ntrows/2;
			lp = wp->w_dotp;
			while (i!=0 && lback(lp)!=wp->w_bufp->b_linep) {
				--i;
				lp = lback(lp);
			}
			wp->w_linep = lp;
			wp->w_skip = 0;
			wp->w_flag |= WFHARD;
		out:
			if (fold_a && wp->w_linep != wp->w_bufp->b_linep) { LINE *p;
			    for (p=lback(wp->w_linep); p!=wp->w_bufp->b_linep; p=lback(p)) {
			        if (LSE(p)) break; if (LSA(p)) { wp->w_linep=p; break; } } }
			lp = wp->w_linep;
			i  = wp->w_toprow;
			if ((wp->w_flag&(WFEDIT|WFHARD)) != 0) {
				hflag = TRUE;
				int skip = wp->w_skip;
				while (i < wp->w_toprow+wp->w_ntrows) {
					if (lp != wp->w_bufp->b_linep) {
						if (fold_a && LSA(lp)) {
							vscreen[i]->v_color=CTEXT; vscreen[i]->v_flag|=(VFCHG|VFHBAD); vtmove(i,0);
							{const char*m="[+ click to expand a-loaded]";while(*m)vtputc(*m++);}
							vteeol(); hl_line(vscreen[i],ncol); i++; FSKIP(lp,wp->w_bufp);
						} else {
						i = wrap_render(lp, i, wp->w_toprow+wp->w_ntrows, wp, skip);
						skip = 0;
						lp = lforw(lp);
						}
					} else {
						vscreen[i]->v_color = CTEXT;
						vscreen[i]->v_flag |= (VFCHG|VFHBAD);
						vtmove(i, 0); vteeol();
						hl_line(vscreen[i], ncol);
						hl_sel(vscreen[i], lp, ncol, wp);
						if(i==0){int bi;for(bi=0;bi<7;bi++)vscreen[0]->v_attr[ncol-31+bi]=HL_NUM;for(bi=0;bi<6;bi++)vscreen[0]->v_attr[ncol-23+bi]=HL_KW;for(bi=0;bi<10;bi++)vscreen[0]->v_attr[ncol-15+bi]=HL_STR;for(bi=0;bi<3;bi++)vscreen[0]->v_attr[ncol-3+bi]=HL_KW;}
						i++;
					}
				}
			}
			if ((wp->w_flag&WFMODE) != 0)
				modeline(wp);
			wp->w_flag  = 0;
			wp->w_force = 0;
		}
		wp = wp->w_wndp;
	}
	lp = curwp->w_linep;
	currow = curwp->w_toprow - curwp->w_skip;
	while (lp != curwp->w_dotp) {
		currow += wrap_rows(lp);
		lp = lforw(lp);
	}
	{int rr=0, rc=0;
	for (i=0; i<curwp->w_doto; i++) {
		c = lgetc(lp, i);
		int ww = c=='\t' ? (8-(rc&7)) : ISCTRL(c) ? 2 : 1;
		if (rc+ww>ncol-2 && rc>0) { rr++; rc = 0; }
		rc += ww;
	}
	currow += rr;
	curcol = rc;
	if (curcol >= ncol) curcol = ncol-1;}
	if (sgarbf != FALSE) {
		sgarbf = FALSE;
		epresf = FALSE;
		tttop  = HUGE;
		ttbot  = HUGE;
		tthue  = CNONE;
		ttmove(0, 0);
		tteeop();
		for (i=0; i<nrow-1; ++i) {
			uline(i, vscreen[i], &blanks);
			ucopy(vscreen[i], pscreen[i]);
		}
		ttmove(currow, curcol);
		ttflush();
		return;
	}
#if	GOSLING
	if (hflag != FALSE) {
		for (i=0; i<nrow-1; ++i) {
			hash(vscreen[i]);
			hash(pscreen[i]);
		}
		offs = 0;
		while (offs != nrow-1) {
			vp1 = vscreen[offs];
			vp2 = pscreen[offs];
			if (vp1->v_color != vp2->v_color
			||  vp1->v_hash  != vp2->v_hash)
				break;
			uline(offs, vp1, vp2);
			ucopy(vp1, vp2);
			++offs;
		}
		if (offs == nrow-1) {
			ttmove(currow, curcol);
			ttflush();
			return;
		}
		size = nrow-1;
		while (size != offs) {
			vp1 = vscreen[size-1];
			vp2 = pscreen[size-1];
			if (vp1->v_color != vp2->v_color
			||  vp1->v_hash  != vp2->v_hash)
				break;
			uline(size-1, vp1, vp2);
			ucopy(vp1, vp2);
			--size;
		}
		if ((size -= offs) == 0)
			abort();
		setscores(offs, size);
		traceback(offs, size, size, size);
		for (i=0; i<size; ++i)
			ucopy(vscreen[offs+i], pscreen[offs+i]);
		ttmove(currow, curcol);
		ttflush();
		return;
	}
#endif
	for (i=0; i<nrow-1; ++i) {
		vp1 = vscreen[i];
		vp2 = pscreen[i];
		if ((vp1->v_flag&VFCHG) != 0) {
			uline(i, vp1, vp2);
			ucopy(vp1, vp2);
		}
	}
	ttmove(currow, curcol);
	ttflush();
}

static void
ucopy(VIDEO * vvp, VIDEO * pvp)
{
	register int	i;

	vvp->v_flag &= ~VFCHG;
	pvp->v_flag  = vvp->v_flag;
	pvp->v_hash  = vvp->v_hash;
	pvp->v_cost  = vvp->v_cost;
	pvp->v_color = vvp->v_color;
	for (i=0; i<ncol; ++i)
		pvp->v_text[i] = vvp->v_text[i];
}

static void
uline(int row, VIDEO * vvp, VIDEO * pvp)
{
	register char	*cp1;
	register char	*cp2;
	register char	*cp3;
	register char	*cp4;
	register char	*cp5;
	register int	nbflag;

	if (vvp->v_color == CTEXT) {
		int col, ca = -1;
		ttmove(row, 0);
		for (col = 0; col < ncol; col++) {
			if (vvp->v_attr[col] != ca) {
				ca = vvp->v_attr[col];
				if (ca >= 0 && ca <= HL_SEL) {
					const char *s = hl_colors[ca];
					while (*s) ttputc(*s++);
				}
			}
			ttputc(vvp->v_text[col]);
			ttcol = col + 1;
		}
		if (ca != HL_NORM) {
			const char *s = hl_colors[HL_NORM];
			while (*s) ttputc(*s++);
		}
		return;
	}
	cp1 = &vvp->v_text[0];
	cp2 = &pvp->v_text[0];
	while (cp1!=&vvp->v_text[ncol] && cp1[0]==cp2[0]) {
		++cp1;
		++cp2;
	}
	if (cp1 == &vvp->v_text[ncol])
		return;
	nbflag = FALSE;
	cp3 = &vvp->v_text[ncol];
	cp4 = &pvp->v_text[ncol];
	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (cp3[0] != ' ')
			nbflag = TRUE;
	}
	cp5 = cp3;
	if (nbflag==FALSE && vvp->v_color==CTEXT) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;

		if ((int)(cp3-cp5) <= tceeol)
			cp5 = cp3;
	}

	ttmove(row, (int)(cp1-&vvp->v_text[0]));
	ttcolor(vvp->v_color);
	while (cp1 != cp5) {
		ttputc(*cp1++);
		++ttcol;
	}
	if (cp5 != cp3)
		tteeol();
}

static void
modeline(WINDOW * wp)
{
	register char	*cp;
	register int	c;
	register int	n;
	register BUFFER	*bp;

	if (box_msg) return;
	n = wp->w_toprow+wp->w_ntrows;
	vscreen[n]->v_color = CMODE;
	vscreen[n]->v_flag |= (VFCHG|VFHBAD);
	vtmove(n, 0);
	bp = wp->w_bufp;
	if ((bp->b_flag&BFCHG) != 0)
		vtputc('*');
	else
		vtputc(' ');
	n  = 1;
	cp = "e";
	while ((c = *cp++) != 0) {
		vtputc(c);
		++n;
	}
	if (bp->b_bname[0] != 0) {
		vtputc(' ');
		++n;
		cp = &bp->b_bname[0];
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	if (bp->b_fname[0] != 0) {
		vtputc(' ');
		++n;
		cp = "File:";
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
		cp = &bp->b_fname[0];
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	if (curmsgf != FALSE
	&& wp->w_wndp == NULL) {
		while (n < ncol-5-1) {
			vtputc(' ');
			++n;
		}
		cp = "[Msg]";
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	while (n < ncol) {
		vtputc(' ');
		++n;
	}
}

#if	GOSLING

static void
hash(VIDEO * vp)
{
	register int	i;
	register int	n;
	register char	*s;

	if ((vp->v_flag&VFHBAD) != 0) {
		s = &vp->v_text[ncol-1];
		for (i=ncol; i!=0; --i, --s)
			if (*s != ' ')
				break;
		n = ncol-i;
		if (n > tceeol)
			n = tceeol;
		vp->v_cost = i+n;
		for (n=0; i!=0; --i, --s)
			n = (n<<5) + n + *s;
		vp->v_hash = n;
		vp->v_flag &= ~VFHBAD;
	}
}

static void
setscores(int offs, int size)
{
	register SCORE	*sp;
	register int	tempcost;
	register int	bestcost;
	register int	j;
	register int	i;
	register VIDEO	**vp;
	register VIDEO	**pp;
	register SCORE	*sp1;
	register VIDEO	**vbase;
	register VIDEO	**pbase;

	vbase = &vscreen[offs-1];
	pbase = &pscreen[offs-1];
	score[0].s_itrace = 0;
	score[0].s_jtrace = 0;
	score[0].s_cost   = 0;
	sp = &score[1];
	tempcost = 0;
	vp = &vbase[1];
	for (j=1; j<=size; ++j) {
		sp->s_itrace = 0;
		sp->s_jtrace = j-1;
		tempcost += tcinsl;
		tempcost += (*vp)->v_cost;
		sp->s_cost = tempcost;
		++vp;
		++sp;
	}
	sp = &score[NROW];
	tempcost = 0;
	for (i=1; i<=size; ++i) {
		sp->s_itrace = i-1;
		sp->s_jtrace = 0;
		tempcost  += tcdell;
		sp->s_cost = tempcost;
		sp += NROW;
	}
	sp1 = &score[NROW+1];
	pp = &pbase[1];
	for (i=1; i<=size; ++i) {
		sp = sp1;
		vp = &vbase[1];
		for (j=1; j<=size; ++j) {
			sp->s_itrace = i-1;
			sp->s_jtrace = j;
			bestcost = (sp-NROW)->s_cost;
			if (j != size)
				bestcost += tcdell;
			tempcost = (sp-1)->s_cost;
			tempcost += (*vp)->v_cost;
			if (i != size)
				tempcost += tcinsl;
			if (tempcost < bestcost) {
				sp->s_itrace = i;
				sp->s_jtrace = j-1;
				bestcost = tempcost;
			}
			tempcost = (sp-NROW-1)->s_cost;
			if ((*pp)->v_color != (*vp)->v_color
			||  (*pp)->v_hash  != (*vp)->v_hash)
				tempcost += (*vp)->v_cost;
			if (tempcost < bestcost) {
				sp->s_itrace = i-1;
				sp->s_jtrace = j-1;
				bestcost = tempcost;
			}
			sp->s_cost = bestcost;
			++sp;
			++vp;
		}
		++pp;
		sp1 += NROW;
	}
}

static void
traceback(int offs, int size, int i, int j)
{
	register int	itrace;
	register int	jtrace;
	register int	k;
	register int	ninsl;
	register int	ndraw;
	register int	ndell;

	if (i==0 && j==0)
		return;
	itrace = score[(NROW*i) + j].s_itrace;
	jtrace = score[(NROW*i) + j].s_jtrace;
	if (itrace == i) {
		ninsl = 0;
		if (i != size)
			ninsl = 1;
		ndraw = 1;
		while (itrace!=0 || jtrace!=0) {
			if (score[(NROW*itrace) + jtrace].s_itrace != itrace)
				break;
			jtrace = score[(NROW*itrace) + jtrace].s_jtrace;
			if (i != size)
				++ninsl;
			++ndraw;
		}
		traceback(offs, size, itrace, jtrace);
		if (ninsl != 0) {
			ttcolor(CTEXT);
			ttinsl(offs+j-ninsl, offs+size-1, ninsl);
		}
		do {
			k = offs+j-ndraw;
			uline(k, vscreen[k], &blanks);
		} while (--ndraw);
		return;
	}
	if (jtrace == j) {
		ndell = 0;
		if (j != size)
			ndell = 1;
		while (itrace!=0 || jtrace!=0) {
			if (score[(NROW*itrace) + jtrace].s_jtrace != jtrace)
				break;
			itrace = score[(NROW*itrace) + jtrace].s_itrace;
			if (j != size)
				++ndell;
		}
		if (ndell != 0) {
			ttcolor(CTEXT);
			ttdell(offs+i-ndell, offs+size-1, ndell);
		}
		traceback(offs, size, itrace, jtrace);
		return;
	}
	traceback(offs, size, itrace, jtrace);
	k = offs+j-1;
	uline(k, vscreen[k], pscreen[offs+i-1]);
}
#endif
static	FILE	*ffp;

static int
ffropen(char * fn)
{
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

static int
ffwopen(char * fn)
{
	if ((ffp=fopen(fn, "w")) == NULL) {
		eprintf("Cannot open file for writing");
		return (FIOERR);
	}
	return (FIOSUC);
}

static int
ffclose(void)
{
	fclose(ffp);
	return (FIOSUC);
}

static int
ffputline(char * buf, int nbuf)
{
	register int	i;

	for (i=0; i<nbuf; ++i)
		putc(buf[i]&0xFF, ffp);
	putc('\n', ffp);
	if (ferror(ffp) != FALSE) {
		eprintf("Write I/O error");
		return (FIOERR);
	}
	return (FIOSUC);
}

static char	*ffline;
static int	ffcap;

static int
ffgetline(char ** bufp, int * lenp)
{
	int	c, i = 0;

	if (!ffcap) { ffcap = NLINE; ffline = malloc(ffcap); }
	for (;;) {
		c = getc(ffp);
		if (c == '\r') { c = getc(ffp); if (c != '\n') { if (i>=ffcap-1) ffline = realloc(ffline, ffcap*=2); ffline[i++] = '\r'; } }
		if (c==EOF || c=='\n') break;
		if (i >= ffcap-1) ffline = realloc(ffline, ffcap*=2);
		ffline[i++] = c;
	}
	if (c==EOF) { if (ferror(ffp)) { eprintf("File read error"); return FIOERR; } if (!i) return FIOEOF; }
	ffline[i] = 0; *bufp = ffline; *lenp = i;
	return FIOSUC;
}

static int
fbackupfile(char * fname)
{
	register char	*nname;

	if ((nname=malloc(strlen(fname)+1+1)) == NULL)
		return (ABORT);
	(void) strcpy(nname, fname);
	(void) strcat(nname, "~");
	(void) unlink(nname);
	if (rename(fname, nname) < 0) {
		free(nname);
		return (FALSE);
	}
	free(nname);
	return (TRUE);
}

static void
adjustcase(char * fn)
{
}
#include	<signal.h>

static char	*shellp	= NULL;

static int
spawncli(int f, int n, int k)
{
	register int	pid;
	register int	wpid;
	void	(*oqsig)(int);
	void	(*oisig)(int);
	int		status;

	if (shellp == NULL) {
		shellp = getenv("SHELL");
		if (shellp == NULL)
			shellp = getenv("shell");
		if (shellp == NULL)
			shellp = "/bin/sh";
	}
	ttcolor(CTEXT);
	ttnowindow();
	if (strcmp(shellp, "/bin/csh") == 0) {
		if (epresf != FALSE) {
			ttmove(nrow-1, 0);
			tteeol();
			epresf = FALSE;
		}
		ttmove(nrow-2, 0);
	} else {
		ttmove(nrow-1, 0);
		if (epresf != FALSE) {
			tteeol();
			epresf = FALSE;
		}
	}
	ttflush();
	ttclose();
	if (strcmp(shellp, "/bin/csh") == 0)
		kill(0, SIGTSTP);
	else {
		oqsig = signal(SIGQUIT, SIG_IGN);
		oisig = signal(SIGINT,  SIG_IGN);
		if ((pid=fork()) < 0) {
			signal(SIGQUIT, oqsig);
			signal(SIGINT,  oisig);
			eprintf("Failed to create process");
			return (FALSE);
		}
		if (pid == 0) {
			execl(shellp, "sh", "-i", NULL);
			_exit(0);
		}
		while ((wpid=wait(&status))>=0 && wpid!=pid)
			;
		signal(SIGQUIT, oqsig);
		signal(SIGINT,  oisig);
	}
	sgarbf = TRUE;
	ttopen();
	return (TRUE);
}
#ifdef HAVE_CONFIG_H
#include	"config.h"
#else
#define PACKAGE_VERSION "30"
#endif

char	*version[] = {
	"e editor version " PACKAGE_VERSION,
	"Source from REX::USER$A:[CONROY.HACKING.MINIEMACS]",
	NULL
};
int	thisflag;
int	lastflag;
int	curgoal;
BUFFER	*curbp;
WINDOW	*curwp;
BUFFER	*bheadp;
WINDOW	*wheadp;
BUFFER	*blistp;
short	kbdm[NKBDM] = { KCTLX | ')' };
short	*kbdmip;
short	*kbdmop;
char	pat[NPAT];
SYMBOL	*symbol[NSHASH];
SYMBOL	*binding[NKEYS];

int
main(int argc, char * * argv)
{
	register int	c;
	register int	f;
	register int	n;
	register int	mflag;
	char		bname[NBUFN];
	int		tail_flag = 0;

	strcpy(bname, "main");
	while (argc >= 2) {
		if (argc >= 3 && !strcmp(argv[1], "--box")) { box_msg = argv[2]; argv += 2; argc -= 2; }
		else if (!strcmp(argv[1], "--tail")) { tail_flag = 1; argv++; argc--; }
		else if (!strcmp(argv[1], "--nosb")) { nosb = 1; argv++; argc--; }
		else if (!strcmp(argv[1], "--nofold")) { fold_a = 0; argv++; argc--; }
		else if (!strcmp(argv[1], "-r")) { ro_flag = 1; argv++; argc--; }
		else if (argv[1][0] == '+' && argv[1][1]) { start_off = atol(argv[1]+1); argv++; argc--; }
		else if (argc >= 3 && !strcmp(argv[1], "--pos-out")) { pos_out_path = argv[2]; argv += 2; argc -= 2; }
		else break;
	}
	if (argc > 1)
		makename(bname, argv[1]);
	vtinit();
	if (box_msg && ncol > 70) ncol = 70;
	signal(SIGWINCH, sigwinch);
	edinit(bname);
	keymapinit();
	if (box_msg) { if (binding[KCTRL|'@']) binding[KCTRL|'@']->s_nkey--; if (binding[KCTRL|'M']) binding[KCTRL|'M']->s_nkey--; binding[KCTRL|'@'] = binding[KCTRL|'M'] = binding[KCTRL|'D']; if (binding[KCTRL|'D']) binding[KCTRL|'D']->s_nkey += 2; }
	if (argc > 1) { update(); readin(argv[1]); } else filldir(".");
	if (tail_flag) { LINE*lp; for(lp=lforw(curbp->b_linep);lforw(lp)!=curbp->b_linep;lp=lforw(lp));
	    curwp->w_dotp=lp; curwp->w_doto=llength(lp); curwp->w_flag|=WFHARD; }
	if (start_off >= 0) {
		LINE *lp = lforw(curbp->b_linep); long off = start_off;
		while (lp != curbp->b_linep && off > llength(lp)) { off -= llength(lp) + 1; lp = lforw(lp); }
		if (lp != curbp->b_linep) { curwp->w_dotp = lp; curwp->w_doto = (int)off; curwp->w_flag |= WFHARD; }
	}
	lastflag = 0;
loop:
	if(resized){resized=0;refresh(0,0,0);}
	{int nb=0;if(rh>=rt)ioctl(0,FIONREAD,&nb);if(rh>=rt&&!nb&&!pmode){update();
	if (box_msg) {
		int sr=ttrow, sc=ttcol, i, ml=(int)strlen(box_msg), cc=0;
		#define BX ttputc(0xE2),ttputc(0x94),ttputc(0x80)
		if (ml > ncol-4) ml = ncol-4;
		ttcolor(CMODE); ttmove(0,0);
		BX; cc++; ttputc(' '); cc++;
		for (i=0; i<ml; i++) { ttputc((unsigned char)box_msg[i]); cc++; }
		ttputc(' '); cc++;
		while (cc<ncol) { BX; cc++; }
		ttmove(nrow-2,0);
		for (cc=0; cc<ncol; cc++) BX;
		#undef BX
		ttcol=ncol; ttcolor(CTEXT);
		ttmove(sr,sc); ttflush();
	}
	}}
	c = getkey();
	if (epresf != FALSE) {
		eerase();
		update();
	}
	f = FALSE;
	n = 1;
	if (c == (KCTRL|'U')) {
		f = TRUE;
		n = 4;
		while ((c=getkey()) == (KCTRL|'U'))
			n *= 4;
		if ((c>='0' && c<='9') || c=='-') {
			if (c == '-') {
				n = 0;
				mflag = TRUE;
			} else {
				n = c - '0';
				mflag = FALSE;
			}
			while ((c=getkey())>='0' && c<='9')
				n = 10*n + c - '0';
			if (mflag != FALSE)
				n = -n;
		}
	}
	if (kbdmip != NULL) {
		if (c!=(KCTLX|')') && kbdmip>&kbdm[NKBDM-6]) {
			ctrlg(FALSE, 0, KRANDOM);
			goto loop;
		}
		if (f != FALSE) {
			*kbdmip++ = (KCTRL|'U');
			*kbdmip++ = n;
		}
		*kbdmip++ = c;
	}
	execute(c, f, n);
	goto loop;
}

static int
execute(int c, int f, int n)
{
	register SYMBOL	*sp;
	register int	status;

	if ((sp=binding[c]) != NULL) {
		thisflag = 0;
		status = (*sp->s_funcp)(f, n, c);
		lastflag = thisflag;
		return (status);
	}
	lastflag = 0;
	return (ABORT);
}

static void
edinit(char * bname)
{
	register BUFFER	*bp;
	register WINDOW	*wp;

	bp = bfind(bname, TRUE);
	blistp = bcreate("");
	wp = (WINDOW *) malloc(sizeof(WINDOW));
	if (bp==NULL || wp==NULL || blistp==NULL)
		abort();
	curbp  = bp;
	wheadp = wp;
	curwp  = wp;
	wp->w_wndp  = NULL;
	wp->w_bufp  = bp;
	bp->b_nwnd  = 1;
	wp->w_linep = bp->b_linep;
	wp->w_dotp  = bp->b_linep;
	wp->w_doto  = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = box_msg ? 1 : 0;
	wp->w_ntrows = box_msg ? nrow-3 : nrow-2;
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;
}

static int
undo(int f, int n, int k)
{int c;if(!ut)return FALSE;c=uc[--ut&2047];ul=1;if(c>0){backchar(FALSE,1,KRANDOM);ldelete(1,FALSE);}else if(!c){backchar(FALSE,1,KRANDOM);ldelnewline();}else if(c>-256)linsert(1,-c);else lnewline();ul=0;lchange(WFHARD);return TRUE;}
static int
jeffexit(int f, int n, int k)
{
	if ((curbp->b_flag&BFCHG) != 0)
		return (filesave(f, n, KRANDOM));
	return (spawncli(f, n, KRANDOM));
}

static int
quit(int f, int n, int k)
{
	if (box_msg && (curbp->b_flag & BFCHG) && filesave(0,0,0) != TRUE) return FALSE;
	write_pos();
	vttidy();
	exit(GOOD);
}

static void
write_pos(void)
{
	long total = 0;
	LINE *p;
	FILE *f;
	if (!pos_out_path) return;
	for (p = lforw(curbp->b_linep); p != curwp->w_dotp && p != curbp->b_linep; p = lforw(p))
		total += llength(p) + 1;
	if (p == curwp->w_dotp) total += curwp->w_doto;
	f = fopen(pos_out_path, "w");
	if (f) { fprintf(f, "%ld\n", total); fclose(f); }
}

/* speak_pid tracks the PID of the most recent `a say` child so [STOP] / Ctrl-Y can kill it.
 * Child setpgid()s into its own group so kill(-pid, SIGKILL) takes out uvx/python/ffmpeg/ffplay too. */
static pid_t speak_pid = 0;
static int
speak_running(void)
{
	int s;
	if (speak_pid <= 0) return 0;
	if (waitpid(speak_pid, &s, WNOHANG) == speak_pid) { speak_pid = 0; return 0; }
	if (kill(speak_pid, 0) < 0) { speak_pid = 0; return 0; }
	return 1;
}
static int
stop_speak(int f, int n, int k)
{
	if (speak_pid > 0) { kill(-speak_pid, SIGKILL); speak_pid = 0; }
	return TRUE;
}
/* speak current line via `a say` (background, doesn't block editor) */
static int
speak_line(int f, int n, int k)
{
	LINE *lp = curwp->w_dotp;
	int len = llength(lp);
	pid_t p;
	if (len <= 0) return TRUE;
	if (speak_pid > 0) { kill(-speak_pid, SIGKILL); speak_pid = 0; }
	p = fork();
	if (p == 0) {
		setpgid(0, 0);
		{ char *text = malloc((size_t)len + 1);
		  memcpy(text, lp->l_text, (size_t)len); text[len] = 0;
		  { char *args[4];
		    args[0] = "a"; args[1] = "say"; args[2] = text; args[3] = NULL;
		    execvp("a", args); }
		}
		_exit(127);
	}
	speak_pid = p;
	return TRUE;
}

static int
ctlxlp(int f, int n, int k)
{
	if (kbdmip!=NULL || kbdmop!=NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	eprintf("[Start macro]");
	kbdmip = &kbdm[0];
	return (TRUE);
}

static int
ctlxrp(int f, int n, int k)
{
	if (kbdmip == NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	eprintf("[End macro]");
	kbdmip = NULL;
	return (TRUE);
}

static int
ctlxe(int f, int n, int k)
{
	register int	c;
	register int	af;
	register int	an;
	register int	s;

	if (kbdmip!=NULL || kbdmop!=NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	if (n <= 0)
		return (TRUE);
	do {
		kbdmop = &kbdm[0];
		do {
			af = FALSE;
			an = 1;
			if ((c = *kbdmop++) == (KCTRL|'U')) {
				af = TRUE;
				an = *kbdmop++;
				c  = *kbdmop++;
			}
			s = TRUE;
		} while (c!=(KCTLX|')') && (s=execute(c, af, an))==TRUE);
		kbdmop = NULL;
	} while (s==TRUE && --n);
	return (s);
}

static int
ctrlg(int f, int n, int k)
{
	ttbeep();
	if (kbdmip != NULL) {
		kbdm[0] = (KCTLX|')');
		kbdmip  = NULL;
	}
	return (ABORT);
}

static int
showversion(int f, int n, int k)
{
	register char	**cpp;
	register char	*cp;

	cpp = &version[0];
	while ((cp = *cpp++) != NULL) {
		if (writemsg(cp) == FALSE)
			return (FALSE);
	}
	if (f != FALSE)
		return (TRUE);
	return (readmsg(0, 1, KRANDOM));
}
