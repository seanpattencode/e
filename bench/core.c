/* bench/core.c — e startup + keystroke latency via pty
 * build: cc -O2 -o bench/core bench/core.c -lutil
 * run:   sh e.c && bench/core
 *
 * Measures real round-trip: write keystroke → poll for output.
 * Includes 2 kernel context switches (~10-100μs), same overhead
 * as Android pty approach.  Direct (no pty) is strictly faster.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pty.h>
#include <time.h>
#include <poll.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define N	200
#define TF	"/tmp/_bench_e.txt"

static long us(struct timespec *a, struct timespec *b){
	return (b->tv_sec-a->tv_sec)*1000000L+(b->tv_nsec-a->tv_nsec)/1000;
}

static void drain(int fd){
	char b[4096];
	while(read(fd,b,sizeof b)>0);
}

static int await(int fd){
	struct pollfd pf={fd,POLLIN,0};
	return poll(&pf,1,2000)>0;
}

int main(void){
	int m,n=0; pid_t pid;
	struct winsize ws={24,80,0,0};
	struct timespec t0,t1;
	long times[N],sum=0;

	{FILE *f=fopen(TF,"w");
	for(int i=0;i<100;i++)fprintf(f,"line %d the quick brown fox jumps\n",i);
	fclose(f);}

	/* --- startup: fork → first output byte --- */
	clock_gettime(CLOCK_MONOTONIC,&t0);
	pid=forkpty(&m,NULL,NULL,&ws);
	if(!pid){execl("./e","e",TF,NULL);_exit(1);}
	if(pid<0){perror("forkpty");return 1;}
	fcntl(m,F_SETFL,fcntl(m,F_GETFL)|O_NONBLOCK);
	await(m);
	clock_gettime(CLOCK_MONOTONIC,&t1);
	long startup=us(&t0,&t1);
	drain(m);
	usleep(20000);

	/* --- keystroke latency: type char → first output byte --- */
	/* send 'x' then backspace each iteration to keep buffer stable */
	for(int i=0;i<N;i++){
		drain(m);
		clock_gettime(CLOCK_MONOTONIC,&t0);
		if(write(m,"x",1)!=1)break;
		if(!await(m)){fprintf(stderr,"timeout i=%d\n",i);break;}
		clock_gettime(CLOCK_MONOTONIC,&t1);
		drain(m);
		times[n]=us(&t0,&t1);
		sum+=times[n];
		n++;
		/* undo: backspace to keep line short */
		(void)write(m,"\x7f",1);
		await(m); drain(m);
	}
	/* sort */
	for(int i=0;i<n;i++){int mi=i;
		for(int j=i+1;j<n;j++)if(times[j]<times[mi])mi=j;
		long t=times[i];times[i]=times[mi];times[mi]=t;}

	printf("startup    %5ld us  %s\n",startup,startup<1000?"< 1ms  OK":"FAIL");
	if(n>0)
		printf("keystroke  min=%-4ld p50=%-4ld avg=%-4ld p99=%-4ld max=%-4ld us  %s  (n=%d)\n",
			times[0],times[n/2],sum/n,times[n*99/100],times[n-1],
			times[n*99/100]<1000?"p99 < 1ms  OK":"p99 FAIL",n);

	(void)write(m,"\x04",1);
	usleep(50000);kill(pid,SIGTERM);waitpid(pid,NULL,0);
	close(m);unlink(TF);
	return 0;
}
