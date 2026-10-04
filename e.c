#if 0
# sh e.c [build|install|debug|clean] — self-compiling editor
[ -z "$BASH_VERSION" ] && exec bash "$0" "$@"
set -e; D="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CC=$(compgen -c clang- 2>/dev/null|grep -xE 'clang-[0-9]+'|sort -t- -k2 -rn|head -1)||CC=""
[[ -z "$CC" ]]&&for c in clang gcc;do command -v $c &>/dev/null&&CC=$c&&break;done
[[ -z "$CC" ]]&&echo "no C compiler"&&exit 1
W="-std=gnu89 -Werror -Weverything -Wno-padded -Wno-disabled-macro-expansion -Wno-reserved-id-macro -Wno-declaration-after-statement -Wno-c99-extensions -Wno-gcc-compat -Wno-unsafe-buffer-usage -Wno-used-but-marked-unused -Wno-unused-parameter -Wno-sign-conversion -Wno-shorten-64-to-32 -Wno-cast-qual -Wno-implicit-int-conversion -Wno-unused-function -Wno-unused-result -Wno-implicit-void-ptr-cast --system-header-prefix=/usr/include"
H="-fstack-protector-strong -ftrivial-auto-var-init=zero -D_FORTIFY_SOURCE=2"
[[ "$(uname -m)" == x86_64 ]]&&H+=" -fstack-clash-protection -fcf-protection"
case "${1:-build}" in
build)   $CC $W $H -fsyntax-only "$D/e.c"& B="-std=gnu89 -O3 -march=native -flto -w";{ command -v musl-gcc>/dev/null&&[[ "$(uname)" == Linux ]]&&musl-gcc -std=gnu11 -D_GNU_SOURCE -O3 -march=native -flto -w -static -o "$D/e" "$D/e.c" 2>/dev/null;}||{ [[ "$(uname)" == Linux ]]&&$CC $B -static -o "$D/e" "$D/e.c" 2>/dev/null;}||$CC $B -o "$D/e" "$D/e.c";;  # musl static CRT ~8us vs glibc ~110us → startup hits the raw-exec/asm floor (easm.s); falls back musl→clang-static→clang-dyn
debug)   $CC $W $H -Wl,-z,relro,-z,now -O1 -g -fsanitize=address,undefined,integer -o "$D/e" "$D/e.c";;
install) sh "$D/e.c"&&mkdir -p "$HOME/.local/bin"&&ln -sf "$D/e" "$HOME/.local/bin/e"&&echo "✓ ~/.local/bin/e";;
clean)   rm -f "$D/e";;
*)       echo "Usage: sh e.c [build|install|debug|clean]";;
esac
exit 0
#endif
#include <time.h>
#define	NROW	66
#define	NCOL	132
enum{KUP=0x81,KDOWN,KLEFT,KRIGHT,KFIND,KINSERT,KREMOVE,KSELECT,KPREV,KNEXT};
#include	<stdio.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<sys/wait.h>
#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif
#include	<sys/select.h>
#include	<signal.h>
static int dirmode,pmode;
static char*pick_out; /* --pick <out>: Enter on a file writes its abs path there and exits (portal/attach) */
static unsigned char rbuf[4096];static int rh,rt;
static volatile sig_atomic_t resized;	/* SIGWINCH flag; declared before ttgetc which reflows on it */
static char dirsrch[1024];static int dirsl;
static int uc[2048],ut,ul;
static char *box_msg;
static int wq_flag;	/* -w: write changes on quit/ESC — flow quick-edit mode */
static int fold_a=1;
/* read-only / bookmark-mode globals (for `a book read` integration) */
static int ro_flag;
static long start_off=-1;
static const char *pos_out_path;
static const char *start_pat;	/* +/pattern: open at first line containing pattern */
#define LSA(lp) (llength(lp)>=12&&!memcmp((lp)->l_text,"## a-loaded ",12))
#define LSE(lp) (llength(lp)>=15&&!memcmp((lp)->l_text,"## a-loaded-end",15))
#define FSKIP(lp,bp) do{lp=lforw(lp);while(lp!=(bp)->b_linep&&!LSE(lp))lp=lforw(lp);if(lp!=(bp)->b_linep)lp=lforw(lp);}while(0)
#define	NFILEN	1024
#define	NPAT	80
enum{FALSE,TRUE,ABORT};
enum{CFCPCN=1,CFKILL=2};	/* last command was a line move / a kill */
enum{CNONE,CTEXT,CMODE};
#define	KCHAR	0x00FF
#define	KCTRL	0x0100
#define	_U	0x02
#define	_L	0x04
#define	ISCTRL(c)	((unsigned char)(c)<0x20||(c)==0x7F)
#define	ISUPPER(c)	isupper((unsigned char)(c))
#define	ISLOWER(c)	islower((unsigned char)(c))
#define	TOUPPER(c)	((c)-0x20)
#define	TOLOWER(c)	((c)+0x20)
typedef int (*KFN)(int);
typedef struct{struct LINE*w_linep,*w_dotp,*w_markp;short w_doto,w_marko;char w_toprow,w_ntrows,w_flag;int w_skip;}WINDOW;	/* w_skip: wrap rows scrolled off above w_linep */
enum{WFMOVE=2,WFEDIT=4,WFHARD=8,WFMODE=16};
typedef struct{struct LINE*b_linep;char b_flag,b_fname[NFILEN];}BUFFER;
#define	BFCHG	0x01
typedef struct{struct LINE*r_linep;short r_offset;int r_size;}REGION;
typedef struct LINE{struct LINE*l_fp,*l_bp;short l_size,l_used;char l_text[1];}LINE;
#define	lforw(lp)	((lp)->l_fp)
#define	lback(lp)	((lp)->l_bp)
#define	lgetc(lp, n)	((lp)->l_text[(n)]&0xFF)
#define	lputc(lp, n, c)	((lp)->l_text[(n)]=(c))
#define	llength(lp)	((lp)->l_used)
static int thisflag,lastflag,curgoal,epresf,sgarbf=TRUE,nrow,ncol,ttrow=-1,ttcol=-1,tthue,vtrow,vtcol;static WINDOW*curwp;static BUFFER*curbp;static char pat[NPAT];static KFN binding[2048];
static int lgen; /* bumps when lines are added/removed — invalidates pos cache */
static char pos_str[5]="All ";	/* top-bar pagination readout: Top/Bot/All/NN%% */
static struct timespec opt0;static char opstr[12]="0.0000ms";	/* opt0: when this operation began (main(), or the read() that delivered its keys) */
static LINE *fe_lp; static int fe_wr;	/* dot line + wrap rows at last update — WFEDIT fast path */
#include	<termios.h>
#include	<sys/ioctl.h>
#define	NOBUF	65536
static char	obuf[NOBUF];
static int	nobuf;
static struct termios oldtty,newtty;
static struct timespec fmt,tz;	/* mtime at load; ttgetc polls it (100ms, 0.6µs/stat) and reloads on change — inotify cost ~1.5ms of exit teardown */
#define MT(a,b) ((a).tv_sec!=(b).tv_sec||(a).tv_nsec!=(b).tv_nsec)
#define	ESC	0x1B
#include <stdarg.h>
#define	NBLOCK	16
#define	KBLOCK	256
static char*kbufp;static int kused,ksize;
typedef struct{char*n;char d;long m;}Dent; /* n -> dpool, full NAME_MAX (n[64] broke Enter on long names); m: mtime, -1 until needed */
#define DENTMAX 16384	/* 512 hid files in big dirs (658-file ~/Downloads); the recursive find shares this room */
static Dent dents[DENTMAX];static char dpool[DENTMAX*96];static int dcnt;static short dview[DENTMAX];static int dvn;
#define DROWS 500	/* rows built per view: every match is counted, but nobody scrolls past a few hundred — building 16k lines cost 5ms a keystroke */
static int dall,rdone,rcut,dcut,dpo,dhere,dbelow;	/* dcnt = this folder; dents[dcnt..dall) = the tree below it; rcut = the walk hit a limit; dpo = dpool bump offset */ /* buffer rows = the filtered view (dview: row->dents) */
static int dhdr; /* 1 = a wrapping cwd header row precedes the entries */
enum{SRCH_FORW=-1,SRCH_BACK=-2,SRCH_NOPR=-5};
static int	srch_lastdir=SRCH_NOPR;
typedef struct{short v_flag,v_color;char v_text[NCOL],v_attr[NCOL];}VIDEO;
enum{HL_NORM,HL_KW,HL_STR,HL_CMT,HL_NUM,HL_PRE,HL_SEL,HL_WHITE};
static const char *hl_colors[]={
"\033[m", /* HL_NORM */
"\033[33m", /* HL_KW  yellow */
"\033[32m", /* HL_STR green */
"\033[36m", /* HL_CMT cyan */
"\033[35m", /* HL_NUM magenta */
"\033[31m", /* HL_PRE red/preprocessor */
"\033[7m", /* HL_SEL reverse video */
"\033[37m", /* HL_WHITE */
};
static const char c_kw[]=" auto break case char const continue default do double else enum extern float for goto if int long register return short signed sizeof static struct switch typedef union unsigned void volatile while NULL TRUE FALSE define include ifdef ifndef endif elif undef pragma ";
#define	VFCHG	0x0001
static VIDEO	*vscreen[NROW-1];
static VIDEO	video[NROW-1];
/* `a say` child, own pgid so kill(-pid) takes uvx/python/ffmpeg too */
static pid_t speak_pid=0;
	/* top-bar button */
/* mutual recursion: the only prototypes left */
static int ereply(char* fp, char* buf, int nbuf, ...);
static int forwchar(int k);
static int backdel(int k);
static int backchar(int k);
static int backline(int k);
static void
ttflush(void)
{
if(nobuf!=0) {
write(1, obuf, nobuf);
nobuf=0;
}
}
static void
ttclose(void)
{
write(1,"\033[?1002l\033[?1006l\033[?2004l\033[2J\033[H\033[?1049l",39); ttflush();	/* leave alt-screen last (clear first for terminals lacking 1049) */
tcflush(0, TCIFLUSH); tcsetattr(1, TCSADRAIN, &oldtty);
}
static void
ttputc(int c)
{
if(nobuf>=NOBUF)ttflush();
obuf[nobuf++]=c;
}
static void ttbeep(void){ttputc(7);ttflush();}
static void tts(const char*s){while(*s)ttputc(*s++);}
static void tteeol(void){tts("\033[K");}
static void tteeop(void){tts("\033[J");}
static void
ttmove(int row, int col)
{
char b[16];
if(ttrow!=row||ttcol!=col){snprintf(b,16,"\033[%d;%dH",row+1,col+1);tts(b);ttrow=row;ttcol=col;}
}
static void ttcolor(int color){if(color!=tthue){tts(color==CMODE?"\033[7m":"\033[m");tthue=color;}}
static void sigwinch(int s){(void)s;resized=1;}
static void
ttresize(void)
{
struct winsize ws;
if(ioctl(0,TIOCGWINSZ,&ws)==0&&ws.ws_row&&ws.ws_col){
nrow=ws.ws_row>NROW?NROW:ws.ws_row;
ncol=ws.ws_col>NCOL?NCOL:ws.ws_col;
}
}
static void
ttopen(void)
{
tcgetattr(1, &oldtty);
newtty=oldtty;cfmakeraw(&newtty);
tcsetattr(1, TCSADRAIN, &newtty); tcflush(0, TCIFLUSH);
nrow=24;ncol=80;ttresize();
write(1, "\033[?1049h\033[?1006h\033[?1002h\033[?2004h", 32);	/* alt-screen: tmux then passes PageUp instead of eating it into copy-mode */
}
static void
eerase(void)
{
ttcolor(CTEXT);
ttmove(nrow-1, 0);
tteeol();
ttflush();
epresf=FALSE;
}
static void
eputc(int c)
{
if(ttcol<ncol) {
if(ISCTRL(c)) {
eputc('^');
c ^= 0x40;
}
ttputc(c);
++ttcol;
}
}
static void
eputs(char * s)
{
int	c;
while((c=*s++)!='\0')eputc(c);
}
static void
eputi(int i, int r)
{
int	q;
if((q=i/r)!=0)eputi(q, r);
eputc(i%r+'0');
}
static void
eformat(char * fp, va_list ap)
{
int	c;
while((c=*fp++)!='\0') {
if(c!='%')eputc(c);
else {
c=*fp++;
switch(c) {
case 'd':
eputi(va_arg(ap, int), 10);
break;
case 'o':
eputi(va_arg(ap, int), 8);
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
static void eprintf(char* fp, ...)
{
va_list ap;
va_start(ap, fp);
ttcolor(CTEXT);
ttmove(nrow-1, 0);
eformat(fp, ap);
tteeol();
ttflush();
epresf=TRUE;
va_end(ap);
}
static LINE *
lalloc(int used)
{
int size=used<NBLOCK?NBLOCK:used*2;LINE*lp=malloc(sizeof(LINE)+(size_t)size);
if(!lp){eprintf("Cannot allocate %d bytes",size);return NULL;}
lp->l_size=(short)size;lp->l_used=(short)used;lgen++;return lp;
}
static void
lfree(LINE * lp)
{
WINDOW*wp=curwp;lgen++;
if(wp->w_linep==lp)wp->w_linep=lp->l_fp;
if(wp->w_dotp==lp){wp->w_dotp=lp->l_fp;wp->w_doto=0;}
if(wp->w_markp==lp){wp->w_markp=lp->l_fp;wp->w_marko=0;}
lp->l_bp->l_fp=lp->l_fp;lp->l_fp->l_bp=lp->l_bp;free(lp);
}
static void lchange(int flag){if(!(curbp->b_flag&BFCHG)){flag|=WFMODE;curbp->b_flag|=BFCHG;}curwp->w_flag|=flag;}
static int
linsert(int n, int c)
{
LINE*lp1=curwp->w_dotp,*lp2;int doto=curwp->w_doto;WINDOW*wp=curwp;
if(ro_flag)return FALSE;
if(!ul)uc[ut++&2047]=c;
lchange(WFEDIT);
if(lp1==curbp->b_linep){	/* at the buffer end: a fresh line */
if(doto){eprintf("bug: linsert");return FALSE;}
if(!(lp2=lalloc(n)))return FALSE;
lp2->l_bp=lp1->l_bp;lp2->l_fp=lp1;lp1->l_bp->l_fp=lp2;lp1->l_bp=lp2;
memset(lp2->l_text,c,(size_t)n);curwp->w_dotp=lp2;curwp->w_doto=n;return TRUE;}
if(lp1->l_used+n>lp1->l_size){	/* grow: copy into a bigger line, splice it in */
if(!(lp2=lalloc(lp1->l_used+n)))return FALSE;
memcpy(lp2->l_text,lp1->l_text,(size_t)doto);memcpy(lp2->l_text+doto+n,lp1->l_text+doto,(size_t)(lp1->l_used-doto));
lp2->l_bp=lp1->l_bp;lp2->l_fp=lp1->l_fp;lp1->l_bp->l_fp=lp2;lp1->l_fp->l_bp=lp2;free(lp1);}
else{lp2=lp1;lp2->l_used+=n;memmove(lp1->l_text+doto+n,lp1->l_text+doto,(size_t)(lp1->l_used-n-doto));}
memset(lp2->l_text+doto,c,(size_t)n);
if(wp->w_linep==lp1)wp->w_linep=lp2;
if(wp->w_dotp==lp1){wp->w_dotp=lp2;wp->w_doto+=n;}
if(wp->w_markp==lp1){wp->w_markp=lp2;if(wp->w_marko>doto)wp->w_marko+=n;}
return TRUE;
}
static int
lnewline(void)
{
LINE*lp1=curwp->w_dotp,*lp2;int doto=curwp->w_doto;WINDOW*wp=curwp;
if(!ul)uc[ut++&2047]=0;
lchange(WFHARD);
if(!(lp2=lalloc(doto)))return FALSE;
memcpy(lp2->l_text,lp1->l_text,(size_t)doto);memmove(lp1->l_text,lp1->l_text+doto,(size_t)(lp1->l_used-doto));lp1->l_used-=doto;
lp2->l_bp=lp1->l_bp;lp1->l_bp=lp2;lp2->l_bp->l_fp=lp2;lp2->l_fp=lp1;
if(wp->w_linep==lp1)wp->w_linep=lp2;
if(wp->w_dotp==lp1){if(wp->w_doto<doto)wp->w_dotp=lp2;else wp->w_doto-=doto;}
if(wp->w_markp==lp1){if(wp->w_marko<doto)wp->w_markp=lp2;else wp->w_marko-=doto;}
return TRUE;
}
static int
ldelnewline(void)
{
LINE*lp1=curwp->w_dotp,*lp2=lp1->l_fp,*lp3;WINDOW*wp=curwp;
if(lp2==curbp->b_linep){if(!lp1->l_used)lfree(lp1);return TRUE;}
if(lp2->l_used<=lp1->l_size-lp1->l_used){	/* fits: append the next line onto this one */
memcpy(lp1->l_text+lp1->l_used,lp2->l_text,(size_t)lp2->l_used);
if(wp->w_linep==lp2)wp->w_linep=lp1;
if(wp->w_dotp==lp2){wp->w_dotp=lp1;wp->w_doto+=lp1->l_used;}
if(wp->w_markp==lp2){wp->w_markp=lp1;wp->w_marko+=lp1->l_used;}
lp1->l_used+=lp2->l_used;lp1->l_fp=lp2->l_fp;lp2->l_fp->l_bp=lp1;free(lp2);return TRUE;}
if(!(lp3=lalloc(lp1->l_used+lp2->l_used)))return FALSE;
memcpy(lp3->l_text,lp1->l_text,(size_t)lp1->l_used);memcpy(lp3->l_text+lp1->l_used,lp2->l_text,(size_t)lp2->l_used);
lp3->l_bp=lp1->l_bp;lp3->l_fp=lp2->l_fp;lp1->l_bp->l_fp=lp3;lp2->l_fp->l_bp=lp3;
if(wp->w_linep==lp1||wp->w_linep==lp2)wp->w_linep=lp3;
if(wp->w_dotp==lp1)wp->w_dotp=lp3;else if(wp->w_dotp==lp2){wp->w_dotp=lp3;wp->w_doto+=lp1->l_used;}
if(wp->w_markp==lp1)wp->w_markp=lp3;else if(wp->w_markp==lp2){wp->w_markp=lp3;wp->w_marko+=lp1->l_used;}
free(lp1);free(lp2);return TRUE;
}
static void kdelete(void){free(kbufp);kbufp=NULL;kused=ksize=0;}
static int
kinsert(int c)
{
if(kused==ksize&&!(kbufp=realloc(kbufp,(size_t)(ksize+=KBLOCK)))){eprintf("Too many kills");return FALSE;}
kbufp[kused++]=(char)c;return TRUE;
}
static int
ldelete(int n, int kflag)
{
LINE*dotp;int doto,chunk,i;WINDOW*wp=curwp;
if(ro_flag)return FALSE;
while(n){
dotp=curwp->w_dotp;doto=curwp->w_doto;
if(dotp==curbp->b_linep)return FALSE;
chunk=dotp->l_used-doto;if(chunk>n)chunk=n;
if(!chunk){if(!ul)uc[ut++&2047]=-256;lchange(WFHARD);if(!ldelnewline()||(kflag&&!kinsert('\n')))return FALSE;n--;continue;}
lchange(WFEDIT);
for(i=0;i<chunk;i++){if(!ul)uc[ut++&2047]=-lgetc(dotp,doto+i);if(kflag&&!kinsert(lgetc(dotp,doto+i)))return FALSE;}
memmove(dotp->l_text+doto,dotp->l_text+doto+chunk,(size_t)(dotp->l_used-doto-chunk));dotp->l_used-=chunk;
if(wp->w_dotp==dotp&&wp->w_doto>=doto){wp->w_doto-=chunk;if(wp->w_doto<doto)wp->w_doto=doto;}
if(wp->w_markp==dotp&&wp->w_marko>=doto){wp->w_marko-=chunk;if(wp->w_marko<doto)wp->w_marko=doto;}
n-=chunk;}
return TRUE;
}
static int
kremove(int n)
{
if(n>=kused)return -1;
return kbufp[n]&0xFF;
}
static char*pickmem(void){static char b[1024];char*h=getenv("HOME");snprintf(b,sizeof b,"%s/.e_pick",h?h:".");return b;}
static void rscan(void)	/* find below this folder: the directories already in dents ARE the queue, so results append breadth-first — this folder's own entries stay ahead of everything deeper. Once per folder, on the first key of a filter */
{
int qi;DIR*d;struct dirent*e;char p[1024];struct timespec t0,t1;
rdone=1;rcut=0;clock_gettime(CLOCK_MONOTONIC,&t0);
for(qi=0;qi<dall;qi++){
clock_gettime(CLOCK_MONOTONIC,&t1);
if(dall>=DENTMAX||dpo>=(int)sizeof dpool-1100||(t1.tv_sec-t0.tv_sec)*1000L+(t1.tv_nsec-t0.tv_nsec)/1000000L>250){rcut=1;break;}	/* a huge or slow tree (network mount) stops the walk rather than freezing the editor; the count line marks the result '+' */
if(!dents[qi].d||!strcmp(dents[qi].n,".."))continue;	/* '..' would walk the tree upwards */
if(!(d=opendir(dents[qi].n)))continue;
while((e=readdir(d))&&dall<DENTMAX&&dpo<(int)sizeof dpool-1100){
if(e->d_name[0]=='.'&&(e->d_type==DT_DIR||!e->d_name[1]||(e->d_name[1]=='.'&&!e->d_name[2])))continue;	/* . .. and dot-dirs (.git) stay out of the walk; dotfiles are still found */
snprintf(p,sizeof p,"%s/%s",dents[qi].n,e->d_name);
dents[dall].d=e->d_type==DT_DIR;
strlcpy(dents[dall++].n=dpool+dpo,p,1024);dpo+=(int)strlen(dpool+dpo)+1;}
closedir(d);}
}
static char sortlab[15]="[sort: a-z]";static int dsort,barmore;	/* 0 name, 1 mtime desc; top-left button + ^U flip it. barmore = [more] overflow open */
static int dentcmp(const void*a,const void*b){Dent*x=(Dent*)a,*y=(Dent*)b;if(dsort){if(x->m!=y->m)return y->m>x->m?1:-1;}else if(x->d!=y->d)return y->d-x->d;return strcasecmp(x->n,y->n);}
static int dishdr(LINE*lp){return dhdr&&lp==lforw(curbp->b_linep);}
static int dentidx(LINE*lp){LINE*l=lforw(curbp->b_linep);int i=0;if(dhdr&&l!=curbp->b_linep)l=lforw(l);for(;l!=lp&&l!=curbp->b_linep&&i<dvn-1;i++)l=lforw(l);return i;}
static char*dname(LINE*lp){return (dvn&&!dishdr(lp))?dents[dview[dentidx(lp)]].n:"";}
static int dpath(void){return dirsl&&(*dirsrch=='/'||*dirsrch=='~'||strchr(dirsrch,'/')!=0);}
static LINE*dadd(char*s,int n){LINE*l=lalloc(n);if(l){l->l_bp=lback(curbp->b_linep);l->l_bp->l_fp=l;l->l_fp=curbp->b_linep;curbp->b_linep->l_bp=l;memcpy(l->l_text,s,n);}return l;}
static int
writeout(char * fn)	/* one write() of the whole buffer (was putc per byte through stdio) */
{
LINE	*lp;long n=0,nl=0;char*m,*p;int fd;
for(lp=lforw(curbp->b_linep);lp!=curbp->b_linep;lp=lforw(lp))n+=llength(lp)+1;
m=p=malloc((size_t)n+1);
for(lp=lforw(curbp->b_linep);lp!=curbp->b_linep;lp=lforw(lp)){memcpy(p,lp->l_text,(size_t)llength(lp));p+=llength(lp);*p++='\n';nl++;}
fd=open(fn,O_WRONLY|O_CREAT|O_TRUNC,0666);
if(fd<0||write(fd,m,(size_t)n)!=n){eprintf("Cannot write %s",fn);free(m);return FALSE;}
close(fd);free(m);eprintf(nl==1?"[Wrote 1 line]":"[Wrote %d lines]",(int)nl);
return TRUE;
}
static int
filesave(int k)
{
WINDOW	*wp;int s;
if((curbp->b_flag&BFCHG)==0)return TRUE;
if(curbp->b_fname[0]==0) {
eprintf("No file name");
return FALSE;
}
if((s=writeout(curbp->b_fname))==TRUE) {
curbp->b_flag &= ~BFCHG;
wp=curwp; {
wp->w_flag |= WFMODE;
}
}
return s;
}
static int
indent(int k)	/* Enter, carrying the line's leading whitespace onto the new one */
{
int nicol=0,c,i;
for(i=0;i<llength(curwp->w_dotp);i++){c=lgetc(curwp->w_dotp,i);if(c!=' '&&c!='\t')break;if(c=='\t')nicol|=7;nicol++;}
if(!lnewline()||((i=nicol/8)&&linsert(i,'\t')==FALSE)||((i=nicol%8)&&linsert(i,' ')==FALSE))return FALSE;
return TRUE;
}
static int
killline(int k)	/* ^K: the rest of the line, or the newline when already at its end */
{
int chunk=llength(curwp->w_dotp)-curwp->w_doto;
if(!(lastflag&CFKILL))kdelete();thisflag|=CFKILL;
return ldelete(chunk?chunk:1,TRUE);
}
static int
clip_osc52(void)
{
int n=kused;FILE *fp;
if(n<=0||n>1000000) return FALSE;
fp=popen("wl-copy 2>/dev/null||xclip -sel c 2>/dev/null||xsel -bi 2>/dev/null||pbcopy 2>/dev/null", "w");
if(!fp) return FALSE;
fwrite(kbufp, 1, (size_t)n, fp);
return pclose(fp)==0;
}
static int setsize(REGION * rp, long size){if(size>0x7FFFFFFF){eprintf("Region is too large");return FALSE;}rp->r_size=(int)size;return TRUE;}
static int
getregion(REGION * rp)	/* the text between dot and mark: line/offset of the earlier end + size */
{
LINE*lp;long s;WINDOW*w=curwp;
if(!w->w_markp){eprintf("No mark in this window");return FALSE;}
if(w->w_dotp==w->w_markp){rp->r_linep=w->w_dotp;rp->r_offset=w->w_doto<w->w_marko?w->w_doto:w->w_marko;rp->r_size=abs(w->w_doto-w->w_marko);return TRUE;}
lp=w->w_dotp;for(s=llength(lp)-w->w_doto+1;lp!=curbp->b_linep;s+=llength(lp)+1){lp=lforw(lp);if(lp==w->w_markp){rp->r_linep=w->w_dotp;rp->r_offset=w->w_doto;return setsize(rp,s+w->w_marko);}}
lp=w->w_markp;for(s=llength(lp)-w->w_marko+1;lp!=curbp->b_linep;s+=llength(lp)+1){lp=lforw(lp);if(lp==w->w_dotp){rp->r_linep=w->w_markp;rp->r_offset=w->w_marko;return setsize(rp,s+w->w_doto);}}
eprintf("Bug: lost mark");return FALSE;
}
static int
killregion(int k)
{
int s;REGION r;
if((s=getregion(&r))!=TRUE)return s;
if(!(lastflag&CFKILL))kdelete();thisflag|=CFKILL;
curwp->w_dotp=r.r_linep;curwp->w_doto=r.r_offset;
if((s=ldelete(r.r_size,TRUE))==TRUE)clip_osc52();return s;
}
static int
copyregion(int k)
{
LINE*lp;int o,s;REGION r;const char*mp=getenv("M_PID");int pid=mp?atoi(mp):0;
if(pid>0){kill(pid,SIGINT);eprintf("[Interrupt sent]");return TRUE;}
if((s=getregion(&r))!=TRUE)return s;
if(!(lastflag&CFKILL))kdelete();thisflag|=CFKILL;
lp=r.r_linep;o=r.r_offset;while(r.r_size--)if(o==llength(lp)){if(!kinsert('\n'))return FALSE;lp=lforw(lp);o=0;}else if(!kinsert(lgetc(lp,o++)))return FALSE;
eprintf(clip_osc52()?"[Copied %d bytes]":"[Copy failed]",kused);return TRUE;
}
static int
gotoeol(int k)
{
curwp->w_doto =llength(curwp->w_dotp);
return TRUE;
}
static int
gotoeob(int k)
{
curwp->w_dotp =curbp->b_linep;
curwp->w_doto =0;
curwp->w_flag |= WFHARD;
return TRUE;
}
static void setgoal(void){int c,i;curgoal=0;for(i=0;i<curwp->w_doto;i++){c=lgetc(curwp->w_dotp,i);if(c=='\t')curgoal|=7;else if(ISCTRL(c))curgoal++;curgoal++;}if(curgoal>=ncol)curgoal=ncol-1;}
static int getgoal(LINE * dlp){int c,col=0,nc,dbo=0;for(;dbo!=llength(dlp);dbo++){c=lgetc(dlp,dbo);nc=col;if(c=='\t')nc|=7;else if(ISCTRL(c))nc++;nc++;if(nc>curgoal)break;col=nc;}return dbo;}
static int
selectall(int k)
{
curwp->w_dotp=lforw(curbp->b_linep);
curwp->w_doto=0;
curwp->w_markp=lback(curbp->b_linep);
curwp->w_marko=llength(lback(curbp->b_linep));
curwp->w_flag |= WFHARD;
eprintf("[All selected]");
return TRUE;
}
static int
setmark(int k)
{
curwp->w_markp=curwp->w_dotp;
curwp->w_marko=curwp->w_doto;
eprintf("[Mark set]");
return TRUE;
}
static int
eq(int bc, int pc)
{
int ibc,ipc;
ibc=bc&0xFF;
ipc=pc&0xFF;
if(ISLOWER(ibc))ibc=TOUPPER(ibc);
if(ISLOWER(ipc))ipc=TOUPPER(ipc);
if(ibc==ipc)return TRUE;
return FALSE;
}
static int
forwsrch(void)	/* forward, case-folded; memchr on the first char; dot lands after the match */
{
LINE*clp=curwp->w_dotp,*tlp;int cbo=curwp->w_doto,tbo,c,n,lo=pat[0]&0xFF,up=lo;char*pp,*p,*q;
if(ISUPPER(lo))lo=TOLOWER(lo);else if(ISLOWER(lo))up=TOUPPER(lo);
for(;clp!=curbp->b_linep;clp=lforw(clp)){n=llength(clp);
if(pat[0]=='\n'){if(cbo>n)goto fail;cbo=n+1;tlp=lforw(clp);tbo=0;}
else{p=cbo<n?memchr(clp->l_text+cbo,lo,(size_t)(n-cbo)):NULL;q=lo!=up&&cbo<n?memchr(clp->l_text+cbo,up,(size_t)(n-cbo)):NULL;
if(!p||(q&&q<p))p=q;if(!p)goto fail;cbo=(int)(p-clp->l_text)+1;tlp=clp;tbo=cbo;}
for(pp=&pat[1];*pp;){if(tlp==curbp->b_linep)goto fail;
if(tbo==llength(tlp)){tlp=lforw(tlp);if(tlp==curbp->b_linep)goto fail;tbo=0;c='\n';}else c=lgetc(tlp,tbo++);
if(!eq(c,*pp++))goto fail;}
curwp->w_dotp=tlp;curwp->w_doto=tbo;curwp->w_flag|=WFMOVE;return TRUE;
fail:cbo=0;}
return FALSE;
}
static int
backsrch(void)	/* pat backward from dot; dot lands at the match start */
{
LINE*clp=curwp->w_dotp,*tlp;int cbo=curwp->w_doto,tbo,c;char*epp=pat+strlen(pat)-1,*pp;
for(;;){if(cbo==0){clp=lback(clp);if(clp==curbp->b_linep)return FALSE;cbo=llength(clp)+1;}
c=--cbo==llength(clp)?'\n':lgetc(clp,cbo);
if(!eq(c,*epp))continue;
tlp=clp;tbo=cbo;for(pp=epp;pp!=pat;){if(tbo==0){tlp=lback(tlp);if(tlp==curbp->b_linep)goto fail;tbo=llength(tlp)+1;}
c=--tbo==llength(tlp)?'\n':lgetc(tlp,tbo);if(!eq(c,*--pp))goto fail;}
curwp->w_dotp=tlp;curwp->w_doto=tbo;curwp->w_flag|=WFMOVE;return TRUE;
fail:;}
}
static int
searchagain(int k)
{
if(srch_lastdir==SRCH_NOPR){eprintf("No last search");return FALSE;}
if(srch_lastdir==SRCH_FORW?forwsrch():backsrch())return TRUE;
eprintf("Not found");return FALSE;
}
static int is_ckw(const char*s,int len){char b[32];if(len>29)return 0;b[0]=' ';memcpy(b+1,s,(size_t)len);b[len+1]=' ';b[len+2]=0;return strstr(c_kw,b)!=0;}
static int
off2col(LINE *lp, int off)
{
int c=0, k;
for(k=0; k<off&&k<llength(lp); k++) {
if(lgetc(lp,k)=='\t') c|=7;
c++;
}
return c;
}
static void
hl_sel(VIDEO *vp, LINE *lp, int cols, WINDOW *wp)	/* inverse the part of lp inside the dot..mark selection */
{
LINE*s1,*s2,*p;int o1,o2,c1=0,c2=cols,in;
if(!wp->w_markp)return;
if(wp->w_dotp==wp->w_markp){if(wp->w_doto==wp->w_marko)return;s1=s2=wp->w_dotp;o1=wp->w_doto<wp->w_marko?wp->w_doto:wp->w_marko;o2=wp->w_doto<wp->w_marko?wp->w_marko:wp->w_doto;}
else{for(p=lforw(wp->w_dotp);p!=curbp->b_linep&&p!=wp->w_markp;p=lforw(p)){}
if(p==wp->w_markp){s1=wp->w_dotp;o1=wp->w_doto;s2=wp->w_markp;o2=wp->w_marko;}else{s1=wp->w_markp;o1=wp->w_marko;s2=wp->w_dotp;o2=wp->w_doto;}}
in=lp==s1||lp==s2;for(p=lforw(s1);!in&&p!=curbp->b_linep&&p!=s2;p=lforw(p))in=p==lp;
if(!in)return;
if(lp==s1)c1=off2col(lp,o1);if(lp==s2)c2=off2col(lp,o2);
for(c1=c1<0?0:c1;c1<c2&&c1<cols;c1++)vp->v_attr[c1]=HL_SEL;
}
static void
hl_line(VIDEO *vp, int cols)	/* C-ish coloring: comments, strings, #directives, keywords, numbers */
{
int i,j,st=HL_NORM;char*t=vp->v_text,*a=vp->v_attr;
for(i=0;i<cols;i++){int c=t[i]&0xFF;
if(st==HL_CMT){a[i]=HL_CMT;continue;}
if(st==HL_STR){a[i]=HL_STR;if(c=='"'&&i>0&&t[i-1]!='\\')st=HL_NORM;continue;}
if(c=='/'&&i+1<cols&&(t[i+1]=='/'||t[i+1]=='*')){st=HL_CMT;a[i]=HL_CMT;continue;}
if(c=='"'){st=HL_STR;a[i]=HL_STR;continue;}
if(c=='#'){int ws;for(j=i+1;j<cols&&t[j]==' ';j++){}for(ws=j;j<cols&&isalpha(t[j]&0xFF);j++){}
if(j>ws&&is_ckw(t+ws,j-ws)){memset(a+i,HL_PRE,(size_t)(j-i));i=j-1;continue;}}
if(isalpha(c)||c=='_'){int k=i;while(i<cols&&(isalnum(t[i]&0xFF)||t[i]=='_'))i++;memset(a+k,is_ckw(t+k,i-k)?HL_KW:HL_NORM,(size_t)(i-k));i--;continue;}
a[i]=isdigit(c)?HL_NUM:HL_NORM;}
}
static void vtinit(void){int i;ttopen();for(i=0;i<NROW-1;i++)vscreen[i]=&video[i];}
static void
vttidy(void)
{
ttcolor(CTEXT);
ttmove(nrow-1, 0);
tteeol();
ttflush();
ttclose();
}
static void pickdone(char*f){char rp[1024];FILE*o;if(!realpath(f,rp))return;if((o=fopen(pick_out,"w"))){fputs(rp,o);fclose(o);}vttidy();exit(0);}
static void
vtmove(int row, int col)
{
vtrow=row;
vtcol=col;
}
static void
vtputc(int c)
{
VIDEO	*vp;
vp=vscreen[vtrow];
if(vtcol<0){vtcol++;return;}
if(vtcol>=ncol) return;
if(c=='\t') {
do {
vtputc(' ');
} while(vtcol<ncol&&(vtcol&0x07)!=0);
} else if(ISCTRL(c)) {
vtputc('^');
vtputc(c ^ 0x40);
} else vp->v_text[vtcol++]=c;
}
static int wpos(LINE*lp,int n,int*cp)	/* wrap rows used by the first n chars of lp; *cp = the column after them */
{int j,col=0,rows=1;for(j=0;j<n;j++){int c=lgetc(lp,j)&0xFF,w=c=='\t'?(8-(col&7)):ISCTRL(c)?2:1;if(col+w>ncol-2&&col>0){rows++;col=0;}col+=w;}if(cp)*cp=col;return rows;}
static int wrap_rows(LINE*lp){return wpos(lp,llength(lp),0);}
static void
uline(int row, VIDEO * vp)	/* one row: attribute runs for text, inverse for the modeline; no shadow-screen diff */
{
int col,ca=-1;
ttmove(row,0);
if(vp->v_color!=CTEXT)ttcolor(vp->v_color);
for(col=0;col<ncol;col++){if(vp->v_color==CTEXT&&vp->v_attr[col]!=ca){ca=vp->v_attr[col];if(ca>=0&&ca<=HL_WHITE)tts(hl_colors[ca]);}ttputc(vp->v_text[col]);}
ttcol=ncol;if(vp->v_color==CTEXT&&ca!=HL_NORM)tts(hl_colors[HL_NORM]);
}
static void
modeline(void)
{
char b[2*NFILEN+16],*bn=strrchr(curbp->b_fname,'/');int n=curwp->w_toprow+curwp->w_ntrows,f=curbp->b_fname[0]&&!dirmode;	/* dir mode shows the full path in the inverse header row instead */
bn=bn&&bn[1]?bn+1:curbp->b_fname;
if(box_msg)return;
vscreen[n]->v_color=CMODE;vscreen[n]->v_flag|=VFCHG;vtmove(n,0);
snprintf(b,sizeof b,"%ce%s%s%s%s",(curbp->b_flag&BFCHG)?'*':' ',bn[0]?" ":"",bn,f?" File:":"",f?curbp->b_fname:"");
for(n=0;b[n];n++)vtputc(b[n]);while(vtcol<ncol)vtputc(' ');
}
static int
execute(int c)
{
int s=ABORT;
if(binding[c]){thisflag=0;s=binding[c](c);}
lastflag=binding[c]?thisflag:0;
return s;
}
static void
edinit(void)	/* one buffer, one window — split-window and use-buffer are gone */
{
static BUFFER b;static WINDOW w;LINE*lp=lalloc(0);
lp->l_fp=lp->l_bp=lp;b.b_linep=lp;curbp=&b;curwp=&w;
w.w_linep=w.w_dotp=lp;w.w_toprow=1;w.w_ntrows=nrow-3;w.w_flag=WFMODE|WFHARD;
}
static void
write_pos(void)
{
long total=0;LINE *p;FILE *f;
if(!pos_out_path) return;
for(p=lforw(curbp->b_linep); p!=curwp->w_dotp&&p!=curbp->b_linep; p=lforw(p))total += llength(p) + 1;
if(p==curwp->w_dotp) total += curwp->w_doto;
f=fopen(pos_out_path, "w");
if(f) { fprintf(f, "%ld\n", total); fclose(f); }
}
static int
quit(int k)
{
if((box_msg||wq_flag)&&(curbp->b_flag&BFCHG)&&filesave(0)!=TRUE) return FALSE;
write_pos();
vttidy();
_exit(0);
}
static int
speak_running(void)
{
int s;
if(speak_pid<=0) return 0;
if(waitpid(speak_pid, &s, WNOHANG)==speak_pid) { speak_pid=0; return 0; }
if(kill(speak_pid, 0)<0) { speak_pid=0; return 0; }
return 1;
}
static void opfmt(long ns)	/* ns -> 8 chars of ms, as many decimals as fit: 100ns steps below 10ms */
{
char b[8];long q=100,f;int d=4,n=0,i,j=0;
for(;d&&ns>=q*100000L;d--)q*=10;	/* past 10ms, 100ms, 1s, 10s: one fewer decimal at each */
f=(ns+q/2)/q;if(f>999999)f=999999;	/* six digits fit; 999999ms reads as off-the-scale */
do{b[n++]=(char)('0'+(int)(f%10));f/=10;}while(f);
while(n<=d)b[n++]='0';	/* leading zeros so the point always keeps a digit on its left */
for(i=n-1;i>=0;i--){opstr[j++]=b[i];if(i==d&&d)opstr[j++]='.';}
opstr[j++]='m';opstr[j++]='s';while(j<8)opstr[j++]=' ';opstr[j]=0;
}
static const struct{short at,w;const char*t;char a;}bar[]={	/* at<0: cols from the right edge; at>=0: from the left. barp() gates paint AND hit-test, so they can't drift */
{-60,7,"[FIND]",HL_STR},{-39,3,"[^]",HL_KW},{-35,3,"[v]",HL_KW},{-23,7,"[SPEAK]",HL_NUM},{-23,6,"[STOP]",HL_KW},{-15,10,"[ADD FILE]",HL_STR},{-3,3,"[X]",HL_KW},{0,14,sortlab,HL_NUM},{-31,6,"[more]",HL_KW},{-53,8,opstr,HL_WHITE},{-44,4,pos_str,HL_NUM},{0,6,"[TERM]",HL_KW}};
#define NBAR (int)(sizeof bar/sizeof bar[0])
static int barp(int i)	/* button i's start col, or -1 when hidden: [SPEAK] only in the open [more] overflow (borrowing [STOP]'s slot), [STOP] only while `a say` is alive, [TERM] (tmux shell split below) file mode only, [FIND] yields it the left edge when too narrow (search stays on C-f); in dirmode the sort button leads the bar and right-anchored items yield to it in thin windows */
{int p=bar[i].at<0?ncol+bar[i].at:bar[i].at;
return p>=(dirmode&&bar[i].at<0?15:0)&&(i==0?dirmode||ncol>65:i==3?barmore&&!speak_running():i==4?speak_running():i==7?dirmode:i==11?!dirmode:1)?p:-1;}
static int barhit(int x)	/* which live button x is on, or -1; hidden or off a narrow terminal is not there to hit */
{int i,p;for(i=0;i<NBAR;i++)if((p=barp(i))>=0&&x>=p&&x<p+bar[i].w)return i;return -1;}
static void tb(int at,const char*t,int a){int i;if(at<0)return;for(i=0;t[i]&&at+i<ncol;i++){vscreen[0]->v_text[at+i]=t[i];vscreen[0]->v_attr[at+i]=(char)a;}}	/* a field starting off-screen is dropped whole: "941ms" for 0.0941ms is worse than none */
static void
vteeol(void)
{
VIDEO*vp=vscreen[vtrow];
while(vtcol<ncol)vp->v_text[vtcol++]=' ';
if(vtrow)return;
{int i;for(i=0;i<NBAR;i++)tb(barp(i),bar[i].t,bar[i].a);}	/* the fill above already blanked the row */
}
static void opend(int r,int c)	/* patched into the frame's own write: no extra syscall, and it covers everything but that write */
{
struct timespec t;
clock_gettime(CLOCK_MONOTONIC,&t);
opfmt((long)(t.tv_sec-opt0.tv_sec)*1000000000L+(long)(t.tv_nsec-opt0.tv_nsec));
if(!box_msg&&ncol>=(dirmode?68:53)){tb(ncol-53,opstr,HL_WHITE);	/* into vscreen too, so the next repaint keeps the same reading; in dirmode it yields to the sort button */
ttmove(0,ncol-53);tts(hl_colors[HL_WHITE]);tts(opstr);tts(hl_colors[HL_NORM]);ttcol=ncol-45;}
ttmove(r,c);ttflush();
}
static int wrap_render(LINE *lp, int row, int max_row, WINDOW *wp, int skip)	/* paint lp from row, wrapping at ncol-2; skip = wrap rows scrolled off above */
{
int j=0,c,col=0,w,hc=dishdr(lp)?CMODE:CTEXT;	/* the cwd header is inverse, like a location bar */
if(row>=max_row)return row;
for(;skip>0&&j<llength(lp);j++){c=lgetc(lp,j);w=c=='\t'?(8-(col&7)):ISCTRL(c)?2:1;if(col+w>ncol-2&&col>0){col=0;if(!--skip)break;}col+=w;}
for(col=0;;col=0){vscreen[row]->v_color=hc;vscreen[row]->v_flag|=VFCHG;vtmove(row,0);
for(;j<llength(lp);j++){c=lgetc(lp,j);w=c=='\t'?(8-(col&7)):ISCTRL(c)?2:1;if(col+w>ncol-2&&col>0)break;vtputc(c);col+=w;}
vteeol();if(hc==CTEXT){hl_line(vscreen[row],ncol);hl_sel(vscreen[row],lp,ncol,wp);}	/* the header is inverse: no syntax/selection coloring on it */
if(j>=llength(lp)||++row>=max_row)return row+(j>=llength(lp)&&row<max_row);}
}
static void
update(void)
{
LINE	*lp;WINDOW	*wp;int i,currow,curcol;
if(curwp->w_markp&&(curwp->w_flag&WFMOVE))curwp->w_flag |= WFHARD;
{static LINE *cl;static int cg=-1,ca,ct;static long cac,ctc;
int a, t, h=curwp->w_ntrows; long ac, tc;
if(cl==curwp->w_linep&&cg==lgen){a=ca;t=ct;ac=cac;tc=ctc;}
else{LINE*p;t=0;a=-1;ac=0;tc=0;
for(p=lforw(curbp->b_linep);p!=curbp->b_linep;p=lforw(p)){if(p==curwp->w_linep){a=t;ac=tc;}tc+=llength(p)+1;t++;}
if(a<0){a=t;ac=tc;}
cl=curwp->w_linep;cg=lgen;ca=a;ct=t;cac=ac;ctc=tc;}
{char*ps=pos_str;
if(ro_flag){long P=(a+h>=t||tc<1)?1000:ac*1000/tc;if(P>=995)strcpy(ps,"100%");else if(P>=100)snprintf(ps,5," %ld%%",P/10);else snprintf(ps,5,"%ld.%ld%%",P/10,P%10);}	/* reader: char-%% of the book (the line readout sits on Top/0%% for ~10k lines) */
else if(t<=h)strcpy(ps," All");else if(a==0)strcpy(ps," Top");else if(a+h>=t)strcpy(ps," Bot");
else{int pc=a*100/(t>1?t-1:1);pc=pc<1?1:pc>99?99:pc;snprintf(ps,5,"%3d%%",pc);}}}
wp=curwp;
if(wp->w_flag!=0) {
lp=wp->w_linep; i=0;
while(i<wp->w_ntrows) {
if(lp==wp->w_dotp) goto out;
if(lp==curbp->b_linep) break;
i += wrap_rows(lp);
lp=lforw(lp);
}
i=wp->w_ntrows/2;lp=wp->w_dotp;
while(i--&&lback(lp)!=curbp->b_linep)lp=lback(lp);
wp->w_linep=lp;
wp->w_skip=0;
wp->w_flag |= WFHARD;
out:
if(fold_a&&wp->w_linep!=curbp->b_linep) { LINE *p;
for(p=lback(wp->w_linep); p!=curbp->b_linep; p=lback(p)) {
if(LSE(p)) break; if(LSA(p)) { wp->w_linep=p; break; } } }
lp=wp->w_linep;
i =wp->w_toprow;
if((wp->w_flag&(WFEDIT|WFHARD))!=0) {
int skip=wp->w_skip;
int fast=!(wp->w_flag&WFHARD)&&!wp->w_markp
&& wp->w_dotp==fe_lp&&wrap_rows(fe_lp)==fe_wr;
while(i<wp->w_toprow+wp->w_ntrows) {
if(lp!=curbp->b_linep) {
if(fold_a&&LSA(lp)) {
if(!fast) {
vscreen[i]->v_color=CTEXT; vscreen[i]->v_flag|=VFCHG; vtmove(i,0);
{const char*m="[+ click to expand a-loaded]";while(*m)vtputc(*m++);}
vteeol(); hl_line(vscreen[i],ncol); }
i++; FSKIP(lp,curbp);
} else if(fast&&lp!=wp->w_dotp) {
i += wrap_rows(lp)-skip;
skip=0;
lp=lforw(lp);
} else {
i=wrap_render(lp, i, wp->w_toprow+wp->w_ntrows, wp, skip);
skip=0;
lp=lforw(lp);
}
} else if(fast) {
i++;
} else {
vscreen[i]->v_color=CTEXT;
vscreen[i]->v_flag |= VFCHG;
vtmove(i, 0); vteeol();
hl_line(vscreen[i], ncol);
hl_sel(vscreen[i], lp, ncol, wp);
i++;
}
}
}
if((wp->w_flag&WFMODE)!=0)modeline();
wp->w_flag =0;
}
if(!box_msg) { vscreen[0]->v_color=CTEXT; vscreen[0]->v_flag |= VFCHG; vtmove(0,0); vteeol(); }	/* paint the dedicated top-bar row (text now starts at row 1) */
lp=curwp->w_linep;
currow=curwp->w_toprow - curwp->w_skip;
while(lp!=curwp->w_dotp) {
currow += wrap_rows(lp);
lp=lforw(lp);
}
currow+=wpos(lp,curwp->w_doto,&curcol)-1;if(curcol>=ncol)curcol=ncol-1;
fe_lp=lp; fe_wr=wrap_rows(lp);
if(sgarbf) {
sgarbf=FALSE;
epresf=FALSE;
tthue =CNONE;
ttmove(0, 0);
tteeop();
for(i=0;i<nrow-1;++i){uline(i,vscreen[i]);vscreen[i]->v_flag&=~VFCHG;}
opend(currow,curcol);
return;
}
for(i=0;i<nrow-1;++i)if(vscreen[i]->v_flag&VFCHG){uline(i,vscreen[i]);vscreen[i]->v_flag&=~VFCHG;}
opend(currow,curcol);
}
static int refresh(int k){int r=nrow,c=ncol;ttresize();if(nrow!=r||ncol!=c){if(nrow<curwp->w_toprow+3){eprintf("Display unusable");return FALSE;}curwp->w_ntrows=nrow-curwp->w_toprow-2;curwp->w_flag|=WFMODE|WFHARD;sgarbf=TRUE;update();eprintf("[New size %d by %d]",nrow,ncol);}else sgarbf=TRUE;return TRUE;}
static int
stop_speak(int k)
{
if(speak_pid>0) { kill(-speak_pid, SIGKILL); speak_pid=0; }
return TRUE;
}
static int
speak_line(int k)	/* ^T: `a say` the current line in the background; [STOP]/^Y kills its whole process group */
{
LINE*lp=curwp->w_dotp;int len=llength(lp);char*t;
if(len<=0)return TRUE;
stop_speak(0);
if(!(speak_pid=fork())){setpgid(0,0);t=malloc((size_t)len+1);memcpy(t,lp->l_text,(size_t)len);t[len]=0;execlp("a","a","say",t,(char*)0);_exit(127);}
return TRUE;
}
static int
ctrlg(int k)
{
ttbeep();
return ABORT;
}
static int
forwpage(int k)	/* k==KPREV pages up; down clamps by wrap rows: the bottom screenful is a fixpoint — no EOF bounce */
{
LINE*lp=curwp->w_linep,*p;int n=curwp->w_ntrows-2,r=curwp->w_ntrows;if(n<=0)n=1;
if(k==KPREV)while(n--&&lback(lp)!=curbp->b_linep)lp=lback(lp);
else{while(n--&&lp!=curbp->b_linep)lp=lforw(lp);
for(p=lp;p!=curbp->b_linep&&r>0;p=lforw(p))r-=wrap_rows(p);
while(r>0&&lback(lp)!=curbp->b_linep&&((r-=wrap_rows(lback(lp)))>=0||lp==curbp->b_linep))lp=lback(lp);}
curwp->w_linep=curwp->w_dotp=lp;curwp->w_doto=0;curwp->w_flag|=WFHARD;return TRUE;
}
static int
eyesno(char * sp)
{
int s;char buf[64];
for(;;){s=ereply("%s [y/n]? ",buf,sizeof buf,sp);if(s==ABORT)return ABORT;if(s&&(buf[0]|32)=='y')return TRUE;if(s&&(buf[0]|32)=='n')return FALSE;}
}
static int bclear(BUFFER * bp){LINE*lp;int s;if((bp->b_flag&BFCHG)&&(s=eyesno("Discard changes"))!=TRUE)return s;bp->b_flag&=~BFCHG;while((lp=lforw(bp->b_linep))!=bp->b_linep)lfree(lp);return TRUE;}
static void dshow(void) /* rows = entries matching dirsrch, middle-elided to ncol-4 (2-char prefix), wrapping past ncol-2 */
{LINE*l;int n,i,lim,w=ncol-4;char s[1100];
bclear(curbp);dvn=dhere=dbelow=0;dcut=rcut;dhdr=0;
if(dpath()){ /* path mode: the path is one wrapping buffer line, cursor at its end */
if((l=dadd(dirsrch,dirsl))){curwp->w_linep=curwp->w_dotp=l;curwp->w_doto=dirsl;
{int wr=wrap_rows(l),nt=curwp->w_ntrows;curwp->w_skip=wr>nt?wr-nt:0;} /* keep the edit point on-screen */
curwp->w_flag|=WFHARD|WFMODE;}return;}
{char cw[1040];if(getcwd(cw,1024)){dadd(cw,(int)strlen(cw));dhdr=1;}} /* cwd as a wrapping header row (never truncated); sort lives on the top bar */
if(dirsl&&!rdone)rscan();	/* the first key of a filter pays for the walk; it is cached until you leave the folder */
lim=dirsl?dall:dcnt;	/* no filter: just this folder. filtering: the whole tree below it, this folder first */
for(i=0;i<lim;i++){
if(dirsl&&!strcasestr(dents[i].n,dirsrch))continue;
if(i<dcnt)dhere++;else dbelow++;
if(dvn>=DROWS){dcut=1;continue;}	/* counted, not listed */
dview[dvn++]=(short)i;
{const char*pre=dents[i].d?"> ":"  ",*nm=dents[i].n;n=(int)strlen(nm);
if(w>=20&&n>w)n=sprintf(s,"%s%.*s~%s",pre,w-13,nm,nm+n-12); /* head + '~' + 12-char tail keeps the extension visible */
else n=sprintf(s,"%s%s",pre,nm);}
dadd(s,n);}
{LINE*h=lforw(curbp->b_linep),*d=(dhdr&&lforw(h)!=curbp->b_linep)?lforw(h):h; /* view top = header; selection starts on the first real entry */
curwp->w_linep=h;curwp->w_dotp=d;curwp->w_doto=0;curwp->w_flag|=WFHARD|WFMODE;}}
static void dbar(void) /* path mode: editable wrapped path + action hint */
{dshow();eprintf("go: Enter=open file/dir");}
static int dloc(int k) /* ^L in the browser = address bar (cwd pre-filled); plain refresh elsewhere */
{if(!dirmode)return refresh(k);
if(getcwd(dirsrch,sizeof dirsrch)){dirsl=(int)strlen(dirsrch);
if(dirsl>1&&dirsl<1023){dirsrch[dirsl++]='/';dirsrch[dirsl]=0;}
dbar();}
return TRUE;}
static void dstat(void)	/* fill mtimes lazily: alphabetical mode never pays for the stats */
{int i;struct stat st;for(i=0;i<dcnt;i++)if(dents[i].m<0)dents[i].m=stat(dents[i].n,&st)?0:(long)st.st_mtime;}
static int
filldir(char *p)
{DIR*d;struct dirent*e;int c=0,o=0;
if(!(d=opendir(p)))return 0;chdir(p);getcwd(curbp->b_fname,NFILEN);
if(pick_out){FILE*g;if((g=fopen(pickmem(),"w"))){fputs(curbp->b_fname,g);fclose(g);}}	/* remember the browsed dir for the next --pick */
while((e=readdir(d))&&c<DENTMAX&&o<(int)sizeof dpool-260){if(e->d_name[0]=='.'&&!e->d_name[1])continue;dents[c].d=e->d_type==DT_DIR;dents[c].m=-1;strlcpy(dents[c++].n=dpool+o,e->d_name,256);o+=(int)strlen(dpool+o)+1;}
closedir(d);dcnt=dall=c;dpo=o;if(dsort)dstat();qsort(dents,c,sizeof(Dent),dentcmp);rdone=rcut=0;dirmode=1;dirsl=0;dirsrch[0]=0;dshow();
eprintf("%d items · type to filter (searches subfolders too) · ^L edit path · ^U sort",c);return 1;}
static int
readin(char * fname)
{
LINE	*lp1,*lp2;
struct stat st;
int	fd,k,nline=0;
long n=0,cap=4096;
char *m,*p,*q,*z;
if(filldir(fname))return TRUE;
fd=open(fname,O_RDONLY);	/* one read: getc-per-byte + 2x lines cost 100µs+ per 5k lines */
if(fd>=0&&!fstat(fd,&st))cap+=st.st_size;
m=malloc(cap);
while(fd>=0&&(k=read(fd,m+n,cap-n))>0){n+=k;if(n==cap)m=realloc(m,cap*=2);}
close(fd);
for(k=0;k<n&&k<512;k++){unsigned c=(unsigned char)m[k];if(c<32&&c!=9&&c!=10&&c!=12&&c!=13)break;}
if((k<n&&k<512)||(n>=4&&!memcmp(m,"%PDF",4))){free(m);if(!fork()){	/* binary: hand to the desktop opener, keep the buffer */
#ifdef __APPLE__
execlp("open","open",fname,(char*)0);
#else
execlp("xdg-open","xdg-open",fname,(char*)0);
#endif
_exit(0);}eprintf("[opened %s]",fname);return TRUE;}
if((k=bclear(curbp))!=TRUE){free(m);return k;}
curbp->b_flag&=~BFCHG;strlcpy(curbp->b_fname,fname,NFILEN);fmt=fd<0?tz:st.st_mtim;
z=m+n;for(p=m;p<z;p=q+1){q=memchr(p,'\n',(size_t)(z-p));if(!q)q=z;k=(int)(q-p);if(k&&q[-1]=='\r')k--;
lp1=malloc(sizeof(LINE)+(size_t)k);lp1->l_size=lp1->l_used=(short)k;memcpy(lp1->l_text,p,(size_t)k);
lp2=lback(curbp->b_linep);lp2->l_fp=lp1;lp1->l_fp=curbp->b_linep;lp1->l_bp=lp2;curbp->b_linep->l_bp=lp1;nline++;}
lgen++;free(m);
eprintf(fd<0?"[New file]":nline==1?"[Read 1 line]":"[Read %d lines]",nline);
curwp->w_linep=curwp->w_dotp=lforw(curbp->b_linep);curwp->w_doto=curwp->w_marko=0;curwp->w_markp=NULL;curwp->w_flag|=WFMODE|WFHARD;
return TRUE;
}
static void dfile(char*x){if(pick_out)pickdone(x);else{dirmode=0;readin(x);}}
static void dopen(LINE*lp){int i;if(!dirmode||!dvn||lp==curbp->b_linep||dishdr(lp))return;i=dview[dentidx(lp)];
if(dents[i].d)filldir(dents[i].n);else dfile(dents[i].n);}
static int dgo(char*q) /* editable path bar: go to a typed/pasted path — file OR folder, '~' expands */
{struct stat st;char x[1024],*h;
if(q[0]=='~'&&(h=getenv("HOME")))snprintf(x,sizeof x,"%s%s",h,q+1);else strlcpy(x,q,sizeof x);
if(stat(x,&st)){eprintf("go: %s — not found",x);return FALSE;}
if(S_ISDIR(st.st_mode))return filldir(x);
dfile(x);return TRUE;}
static int
backdir(int k)
{if(dirmode){filldir("..");}else{char d[NFILEN],*p;strcpy(d,curbp->b_fname);p=strrchr(d,'/');if(p)*p=0;else*d=0;filldir(*d?d:".");}return TRUE;}
static void winch(void)	/* SIGWINCH: resize, rebuild the dir view keeping the selection */
{resized=0;clock_gettime(CLOCK_MONOTONIC,&opt0);refresh(0);if(dirmode){int _i=dentidx(curwp->w_dotp);dshow();while(_i--)curwp->w_dotp=lforw(curwp->w_dotp);curwp->w_flag|=WFMOVE;}update();ttflush();}
static int
ttgetc(void)
{
if(rh<rt)return rbuf[rh++];
for(;;){fd_set r;FD_ZERO(&r);FD_SET(0,&r);struct timeval t={0,100000};struct stat s;
if(select(1,&r,0,0,&t)<0){if(resized)winch();continue;}	/* EINTR from SIGWINCH: reflow now (foot tiles after exec) */
if(FD_ISSET(0,&r)){int n=read(0,rbuf,sizeof rbuf);if(n>0){rh=1;rt=n;clock_gettime(CLOCK_MONOTONIC,&opt0);return rbuf[0];}continue;}	/* the operation starts the moment its bytes arrive, decode included */
if(!dirmode&&!(curbp->b_flag&BFCHG)&&!stat(curbp->b_fname,&s)&&MT(s.st_mtim,fmt)){LINE*lp;clock_gettime(CLOCK_MONOTONIC,&opt0);readin(curbp->b_fname);	/* the reload is its own operation: time it from here */
for(lp=lforw(curbp->b_linep);lforw(lp)!=curbp->b_linep;lp=lforw(lp));
curwp->w_dotp=lp;curwp->w_doto=llength(lp);
curwp->w_flag|=WFHARD;update();ttflush();}}
}
static int
eread(char * fp, char * buf, int nbuf, va_list ap)	/* the echo-line prompt: Enter done, ^G abort, BS/^U edit */
{
int cpos=0,c;
ttcolor(CTEXT);ttmove(nrow-1,0);epresf=TRUE;eformat(fp,ap);tteeol();ttflush();
for(;;){c=ttgetc();
if(c==0x0D){buf[cpos]=0;ttputc(0x0D);ttflush();return buf[0]!=0;}
if(c==0x07){eputc(7);ctrlg(0);ttflush();return ABORT;}
if(c==0x7F||c==0x08||c==0x15){while(cpos){int w=ISCTRL(buf[--cpos])?2:1;while(w--){tts("\b \b");ttcol--;}if(c!=0x15)break;}ttflush();}
else if(cpos<nbuf-1){buf[cpos++]=c;eputc(c);ttflush();}}
}
static int ereply(char* fp, char* buf, int nbuf, ...){va_list ap;int r;va_start(ap,nbuf);r=eread(fp,buf,nbuf,ap);va_end(ap);return r;}
static int
filevisit(int k)
{
char fname[NFILEN];int s;
if((s=ereply("Visit file: ", fname, NFILEN))!=TRUE)return s;
return readin(fname);
}
static void dfind(void)	/* how many matches are in this folder vs below it, and which one is selected */
{
eprintf("find: %s (%d here, %d below%s) -> %s",dirsl?dirsrch:"",dhere,dbelow,dcut?"+":"",dvn?dname(curwp->w_dotp):"no match — Backspace");
}
static void dfilter(void) /* [FIND] button while browsing: prompt a filter — the mouse-only route to narrowing */
{char q[64];if(ereply("filter: ",q,64)!=TRUE)q[0]=0;
strlcpy(dirsrch,q,64);dirsl=(int)strlen(dirsrch);dshow();dfind();}
static int dosort(int k)	/* ^U / top-left sort button: flip name-sort <-> most-recent-first and re-list */
{dsort^=1;strcpy(sortlab,dsort?"[sort: newest]":"[sort: a-z]");
if(!dirmode){eprintf("[sort: %s]",dsort?"most recent":"a-z");return TRUE;}
if(dsort)dstat();qsort(dents,dcnt,sizeof(Dent),dentcmp);dshow();dfind();return TRUE;}
static int
selfinsert(int k)
{
int c=k&KCHAR;
if(dirmode){
if((c=='\t'||((k&KCTRL)&&c=='I'))&&dvn){LINE*n2=lforw(curwp->w_dotp);	/* Tab (KCTRL|'I') = next visible row */
if(n2==curbp->b_linep)n2=lforw(n2);if(dishdr(n2))n2=lforw(n2);	/* wrap-around lands past the path header, on the first real entry */
curwp->w_dotp=n2;curwp->w_doto=0;curwp->w_flag|=WFMOVE;eprintf("find: %s -> %s",dirsl?dirsrch:"(all)",dname(n2));return TRUE;}
if(dirsl<1023){dirsrch[dirsl++]=(char)c;dirsrch[dirsl]=0;}
if(dpath()){dbar();return TRUE;}	/* path bar: type or paste any path */
dshow();dfind();	/* echo where the matches are, and the selected full path (rows are elided) */
return TRUE;}
if((k&KCTRL)&&c>='@'&&c<='_')c-='@';
return linsert(1,c);
}
static int
gotoline(int k)
{
LINE*lp=lforw(curbp->b_linep);int s,n;char buf[32];
if((s=ereply("Goto line: ",buf,sizeof buf))!=TRUE)return s;
if((n=atoi(buf))<=0){eprintf("Bad line");return FALSE;}
for(;n>1;n--){if(lp==curbp->b_linep){eprintf("Line number too large");return FALSE;}lp=lforw(lp);}
curwp->w_dotp=lp;curwp->w_doto=0;curwp->w_flag|=WFMOVE;return TRUE;
}
static int
readpattern(char * prompt)
{
int s;char tpat[NPAT];
s=ereply("%s [%s]: ", tpat, NPAT, prompt, pat);
if(s==TRUE)strcpy(pat, tpat);
else if(s==FALSE&&pat[0]!=0)s=TRUE;
return s;
}
static int bchar(int n)	/* the one movement still taking a count: i-search/replace step by pattern length */
{
LINE*lp;
while(n--){if(curwp->w_doto)curwp->w_doto--;
else{if((lp=lback(curwp->w_dotp))==curbp->b_linep)return FALSE;
curwp->w_dotp=lp;curwp->w_doto=llength(lp);curwp->w_flag|=WFMOVE;}}
return TRUE;
}
static int fchar(int n)
{
while(n--){if(curwp->w_doto!=llength(curwp->w_dotp))curwp->w_doto++;
else{if(curwp->w_dotp==curbp->b_linep)return FALSE;
curwp->w_dotp=lforw(curwp->w_dotp);curwp->w_doto=0;curwp->w_flag|=WFMOVE;}}
return TRUE;
}
static int backchar(int k){return bchar(1);}
static int forwchar(int k){return fchar(1);}
static int
newline(int k)
{
LINE*lp;
if(dirmode){if(dpath())dgo(dirsrch);else dopen(curwp->w_dotp);return TRUE;}
lp=curwp->w_dotp;
if(llength(lp)==curwp->w_doto&&lp!=curbp->b_linep&&!llength(lforw(lp)))return fchar(1);	/* end of a line whose successor is blank: step onto it, don't split */
return lnewline();
}
static int
yank(int k)
{
int c,i,nline=0;LINE*lp;
for(i=0;(c=kremove(i))>=0;i++){if(c=='\n'){if(newline(0)==FALSE)return FALSE;nline++;}else if(linsert(1,c)==FALSE)return FALSE;}
lp=curwp->w_linep;
if(curwp->w_dotp==lp){while(nline--&&lback(lp)!=curbp->b_linep)lp=lback(lp);curwp->w_linep=lp;curwp->w_flag|=WFHARD;}
return TRUE;
}
static int
backdel(int k)
{
int	s;
if(dirmode) { if(dirsl>0)dirsrch[--dirsl]=0;
if(dpath()){dbar();return TRUE;}
dshow(); dfind(); return TRUE; }
if((s=bchar(1))==TRUE)s=ldelete(1,FALSE);
return s;
}
static int
lreplace(int plen, char * st)	/* replace the plen chars before dot with st, keeping the original's case shape */
{
int rlen=(int)strlen(st),rtype=_L,c,doto;
bchar(plen);
c=lgetc(curwp->w_dotp,curwp->w_doto);
if(ISUPPER(c)){rtype=_U|_L;if(curwp->w_doto+1<llength(curwp->w_dotp)&&ISUPPER(lgetc(curwp->w_dotp,curwp->w_doto+1)))rtype=_U;}
doto=curwp->w_doto;
if(plen>rlen)ldelete(plen-rlen,FALSE);else if(plen<rlen&&linsert(rlen-plen,' ')==FALSE)return FALSE;
curwp->w_doto=doto;
while((c=*st++&0xff)!=0){
if((rtype&_U)&&ISLOWER(c))c=TOUPPER(c);
if(rtype==(_U|_L))rtype=_L;
if(c=='\n'){if(curwp->w_doto==llength(curwp->w_dotp))fchar(1);else{ldelete(1,FALSE);lnewline();}}
else if(curwp->w_dotp==curbp->b_linep)linsert(1,c);
else if(curwp->w_doto==llength(curwp->w_dotp)){ldelete(1,FALSE);linsert(1,c);}
else lputc(curwp->w_dotp,curwp->w_doto++,c);}
lchange(WFHARD);return TRUE;
}
static int srch(int dir,int nx)	/* re-find pat from the match start (nx: from the next char); dot restored on failure */
{int p=(int)strlen(pat),o=curwp->w_doto,r;LINE*l=curwp->w_dotp;
if(dir==SRCH_FORW){fchar(nx);bchar(p);r=forwsrch();}
else{bchar(nx);fchar(p);r=backsrch();}
if(!r){curwp->w_dotp=l;curwp->w_doto=o;}return r;}
static int
isearch(int dir)	/* type to search; BS undoes, ^F/^S/^N next, ^R/^B/^P prev, ^Q quotes, ESC/Enter done, ^G restores; other ctrl keys run their command and end it */
{
int c,n=0,ok=1,typed=0,so=curwp->w_doto,sto[NPAT];LINE*sl=curwp->w_dotp,*st[NPAT];	/* the last pattern stays live until a key is typed, so ^F^F repeats it and ESC leaves it for search-again */
for(;;){eprintf("%s%s-search: %s",ok?"":"failing ",dir==SRCH_FORW?"i":"reverse i",pat);update();
c=ttgetc();
if(c==0x0D||c==0x1B){srch_lastdir=dir;eprintf("[Done]");return TRUE;}
if(c==0x07){curwp->w_dotp=sl;curwp->w_doto=so;curwp->w_flag|=WFMOVE;ctrlg(0);return FALSE;}
if(c==0x06||c==0x13||c==0x0E||c==0x12||c==0x02||c==0x10){dir=(c==0x12||c==0x02||c==0x10)?SRCH_BACK:SRCH_FORW;if(pat[0]&&!(ok=srch(dir,1)))ttbeep();continue;}
if(c==0x7F||c==0x08){if(n){n--;pat[n]=0;curwp->w_dotp=st[n];curwp->w_doto=sto[n];curwp->w_flag|=WFMOVE;ok=1;}continue;}
if(c==0x1E||c==0x11)c=ttgetc();
else if(ISCTRL(c)&&c!=0x0A){int r=execute(KCTRL|(c+'@'));curwp->w_flag|=WFMOVE;return r;}
if(!typed){typed=1;n=0;pat[0]=0;}	/* first key typed starts a fresh pattern */
if(n<NPAT-1){st[n]=curwp->w_dotp;sto[n]=curwp->w_doto;pat[n++]=(char)c;pat[n]=0;if(!(ok=srch(dir,0)))ttbeep();}}
}
static int
forwisearch(int k)
{
return (isearch(SRCH_FORW));
}
static int
backisearch(int k)
{
return (isearch(SRCH_BACK));
}
static int qrep(int plen,char*news,LINE**clp){int kl=curwp->w_dotp==*clp;if(lreplace(plen,news)==FALSE)return 0;if(kl)*clp=curwp->w_dotp;return 1;}
static int
queryrepl(int k)
{
int s,nrep=0,plen,cbo;char news[NPAT];LINE*clp;
if((s=readpattern("Old string"))!=TRUE)return s;
if((s=ereply("New string: ",news,NPAT))==ABORT)return s;
if(!s)news[0]=0;
eprintf("Query Replace:  [%s] -> [%s]",pat,news);
plen=(int)strlen(pat);clp=curwp->w_dotp;cbo=curwp->w_doto;
while(forwsrch()==TRUE){
retry:update();
switch(ttgetc()){
case ' ':case ',':if(!qrep(plen,news,&clp))return FALSE;nrep++;break;
case '.':if(!qrep(plen,news,&clp))return FALSE;nrep++;goto stop;
case 0x07:ctrlg(0);goto stop;
case '!':do{if(!qrep(plen,news,&clp))return FALSE;nrep++;}while(forwsrch()==TRUE);goto stop;
case 'n':break;
default:eprintf("<SP>[,] replace, [.] rep-end, [n] don't, [!] repl rest [C-G] quit");goto retry;}}
stop:curwp->w_dotp=clp;curwp->w_doto=cbo;curwp->w_flag|=WFHARD;update();
eprintf(nrep==0?"[No replacements done]":nrep==1?"[1 replacement done]":"[%d replacements done]",nrep);
return TRUE;
}
static int
undo(int k)
{int c;if(!ut)return FALSE;c=uc[--ut&2047];ul=1;if(c>0){bchar(1);ldelete(1,FALSE);}else if(!c){bchar(1);ldelnewline();}else if(c>-256)linsert(1,-c);else lnewline();ul=0;lchange(WFHARD);return TRUE;}
static int
getkbd(void)
{
int c,n;
loop:
c=ttgetc();
if(c==ESC) {
{fd_set r;FD_ZERO(&r);FD_SET(0,&r);struct timeval t={0,50000};
if(rh>=rt&&!select(1,&r,0,0,&t)){quit(0);return 0;}}
c=ttgetc();
if(c=='[') {
c=ttgetc();
if(c=='<') {
int b=0,x=0,y=0,ch,row; LINE *lp;
while((ch=ttgetc())!=';') b=b*10+ch-'0';
while((ch=ttgetc())!=';') x=x*10+ch-'0';
while((ch=ttgetc())!='M'&&ch!='m') y=y*10+ch-'0';
if(b>=64&&b<128){if(!(b&2))forwpage(b&1?0:KPREV);curwp->w_flag|=WFHARD;update();goto loop;}	/* wheel pages; 66/67 (tilt) ignored */
x--; y--; row=y-curwp->w_toprow;
if(b&32)goto loop;
if(y==0&&ch=='M'){int h=barhit(x);
if(h==6){quit(0);goto loop;}
if(h==1){forwpage(KPREV);update();goto loop;}
if(h==2){forwpage(0);update();goto loop;}
if(h==3){speak_line(0);barmore=0;update();goto loop;}
if(h==4){stop_speak(0);goto loop;}
if(h==5){
char fn[NFILEN]="";FILE*fp;
eprintf("[Pick a file...]");update();ttflush();
#ifdef __APPLE__
fp=popen("osascript -e 'tell app \"SystemUIServer\" to activate' -e 'POSIX path of (choose file)' 2>/dev/null","r");
#else
fp=popen("zenity --file-selection 2>/dev/null || kdialog --getopenfilename . 2>/dev/null","r");
#endif
if(fp){if(fgets(fn,NFILEN,fp))fn[strcspn(fn,"\n")]=0;pclose(fp);}
if(fn[0]){readin(fn);sgarbf=TRUE;}else eprintf("[Cancelled]");
goto loop;}
if(h==7){dosort(0);update();goto loop;}
if(h==8){barmore^=1;update();goto loop;}
if(h==11){if(system("tmux split-window -v -l 25% -c \"#{pane_current_path}\""))eprintf("[tmux split failed]");goto loop;}
if(h==0)goto loop;}	/* [FIND] acts on release: on press the release's ESC seq would land inside isearch */
if(y==0&&ch=='m'&&barhit(x)==0){if(dirmode)dfilter();else forwisearch(0);update();goto loop;}	/* [FIND] in the browser = filter prompt, not buffer isearch */
if(row>=0&&row<curwp->w_ntrows) {
if(dirmode){for(lp=curwp->w_linep;lp!=curbp->b_linep;){int wr=wrap_rows(lp);if(row<wr)break;row-=wr;lp=lforw(lp);}} /* wrap-aware: the wrapped header must not offset the clicked row */
else for(lp=curwp->w_linep;row>0&&lp!=curbp->b_linep;row--){if(fold_a&&LSA(lp))FSKIP(lp,curbp);else lp=lforw(lp);}
if(ch=='M'&&LSA(lp)){fold_a=!fold_a;curwp->w_dotp=lp;curwp->w_doto=0;curwp->w_flag|=WFHARD;sgarbf=TRUE;update();goto loop;}
curwp->w_dotp=lp;{int i,cc;for(i=cc=0;i<llength(lp)&&cc<x;cc=lgetc(lp,i++)==9?(cc|7)+1:cc+1){}curwp->w_doto=i;}
if(ch=='M'){if(b>=128&&!(b&32)){backdir(0);}else if(!(b&3)&&!(b&32)&&dirmode)dopen(lp);}
curwp->w_flag|=WFMOVE; update();
}
goto loop;
}
if(c>='A'&&c<='D')return (unsigned char)"\x81\x82\x84\x83"[c-'A'];	/* arrows: A up, B down, C right, D left */
if(c>='0'&&c<='9') {
n=0;
do {
n=10*n + c - '0';
c=ttgetc();
} while(c>='0'&&c<='9');
if(c=='~'&&n>=1&&n<=6)return 0x84+n;	/* \e[1~..6~ = Home Ins Del End PgUp PgDn -> KFIND..KNEXT */
if(c=='~'&&(n==200||n==201)) {pmode=(n==200); if(!pmode){update();ttflush();}}
}
goto loop;
}
if(c=='O'){c=ttgetc();if(c>='A'&&c<='D')return (unsigned char)"\x81\x82\x84\x83"[c-'A'];}
goto loop;	/* ESC+other: nothing is bound to meta keys */
}
return c;
}
static int
getkey(void)
{
int	c;
c=getkbd();
if(c>=0x00&&c<=0x1F)c=KCTRL|(c+'@');
return c;
}
static int forwline(int k){LINE*lp=curwp->w_dotp;if(!(lastflag&CFCPCN))setgoal();thisflag|=CFCPCN;if(lp!=curbp->b_linep)lp=lforw(lp);curwp->w_dotp=lp;curwp->w_doto=getgoal(lp);curwp->w_flag|=WFMOVE;return TRUE;}
static int backline(int k){LINE*lp=curwp->w_dotp;if(!(lastflag&CFCPCN))setgoal();thisflag|=CFCPCN;if(lback(lp)!=curbp->b_linep)lp=lback(lp);curwp->w_dotp=lp;curwp->w_doto=getgoal(lp);curwp->w_flag|=WFMOVE;return TRUE;}
#define	DIRLIST	0
#if	DIRLIST
#endif
static KFN ctl[26]={selectall,backdir,copyregion,quit,gotoeol,forwisearch,gotoline,queryrepl,selfinsert,indent,killline,dloc,newline,forwline,filevisit,backline,quit,backisearch,filesave,speak_line,dosort,yank,quit,killregion,stop_speak,undo};	/* ^A..^Z, VSCode-style; ^U = sort toggle */
static void
keymapinit(void)
{
int i;
for(i=0x20;i<0xFF;i++)if(i<0x7F||i>=0xA0)binding[i]=selfinsert;	/* 0x80-0x9F are the DEC key codes; every other byte inserts itself (UTF-8 bytes included) */
for(i=0;i<26;i++)binding[KCTRL|('A'+i)]=ctl[i];
binding[KCTRL|'@']=setmark;binding[KLEFT]=backchar;binding[KRIGHT]=forwchar;binding[KCTRL|'[']=ctrlg;binding[0x7F]=backdel;binding[KNEXT]=forwpage;binding[KPREV]=forwpage;binding[KSELECT]=gotoeob;binding[KFIND]=searchagain;binding[KINSERT]=yank;binding[KREMOVE]=killregion;binding[KUP]=backline;binding[KDOWN]=forwline;
}
#include	<signal.h>
#ifdef HAVE_CONFIG_H
#include	"config.h"
#else
#endif
int
main(int argc, char * * argv)
{
clock_gettime(CLOCK_MONOTONIC,&opt0);
int c,tail_flag=0;
while(argc>=2) {
if(argc>=3&&!strcmp(argv[1],"--box")){box_msg=argv[2];argv+=2;argc-=2;}
else if(!strcmp(argv[1],"--tail")){tail_flag=1;argv++;argc--;}
else if(!strcmp(argv[1],"--nofold")){fold_a=0;argv++;argc--;}
else if(!strcmp(argv[1],"-r")){ro_flag=1;argv++;argc--;}
else if(!strcmp(argv[1],"-w")){wq_flag=1;argv++;argc--;}
else if(argv[1][0]=='+'&&argv[1][1]=='/'){start_pat=argv[1]+2;argv++;argc--;}
else if(argv[1][0]=='+'&&argv[1][1]){start_off=atol(argv[1]+1);argv++;argc--;}
else if(argc>=3&&!strcmp(argv[1],"--pos-out")){pos_out_path=argv[2];argv+=2;argc-=2;}
else if(argc>=3&&!strcmp(argv[1],"--pick")){pick_out=argv[2];argv+=2;argc-=2;}
else break;
}
vtinit();
if(box_msg&&ncol>70) ncol=70;
{struct sigaction sa;memset(&sa,0,sizeof sa);sa.sa_handler=sigwinch;sigaction(SIGWINCH,&sa,0);}	/* no SA_RESTART: SIGWINCH must interrupt select() */
edinit();
keymapinit();
if(box_msg)binding[KCTRL|'@']=binding[KCTRL|'M']=binding[KCTRL|'D'];
if(pick_out){char md[1024]={0};FILE*g;
if((g=fopen(pickmem(),"r"))){if(fgets(md,1024,g))md[strcspn(md,"\n")]=0;fclose(g);}
if(!(md[0]&&filldir(md)))filldir(argc>1?argv[1]:".");}
else if(argc>1)readin(argv[1]); else filldir(".");
if(tail_flag){LINE*lp;for(lp=lforw(curbp->b_linep);lforw(lp)!=curbp->b_linep;lp=lforw(lp));
curwp->w_dotp=lp;curwp->w_doto=llength(lp);curwp->w_flag|=WFHARD;}
if(start_off>=0){LINE*lp=lforw(curbp->b_linep);long off=start_off;
while(lp!=curbp->b_linep&&off>llength(lp)){off-=llength(lp)+1;lp=lforw(lp);}
if(lp!=curbp->b_linep){curwp->w_dotp=lp;curwp->w_doto=(int)off;curwp->w_flag|=WFHARD;}}
if(start_pat){int pl=(int)strlen(start_pat);LINE*lp;
for(lp=lforw(curbp->b_linep);lp!=curbp->b_linep;lp=lforw(lp)){int i=0;
while(i+pl<=llength(lp)&&memcmp(lp->l_text+i,start_pat,(size_t)pl))i++;
if(i+pl<=llength(lp)){curwp->w_dotp=lp;curwp->w_doto=i;curwp->w_flag|=WFHARD;break;}}}
lastflag=0;
loop:
if(resized)winch();
if(rh>=rt){int nb=0;ioctl(0,FIONREAD,&nb);if(!nb&&!pmode){update();
if(box_msg) {	/* frame the message: ─ rules above and below the text */
int sr=ttrow,sc=ttcol,i,ml=(int)strlen(box_msg);if(ml>ncol-4)ml=ncol-4;
ttcolor(CMODE);ttmove(0,0);tts("─ ");for(i=0;i<ml;i++)ttputc((unsigned char)box_msg[i]);ttputc(' ');for(i=ml+3;i<ncol;i++)tts("─");
ttmove(nrow-2,0);for(i=0;i<ncol;i++)tts("─");ttcol=ncol;ttcolor(CTEXT);ttmove(sr,sc);ttflush();
}
}}
c=getkey();
if(epresf){eerase();update();}
execute(c);
goto loop;
}
