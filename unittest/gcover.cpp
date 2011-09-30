#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
 
#define SIMPLE_WAY
 
void sighandler(int signo) {
#ifdef SIMPLE_WAY   
    exit(signo);
#else   
    extern void __gcov_flush();
    // flush out gcov stats data
    __gcov_flush(); 
    // raise the signal again to crash process
    raise(signo);
#endif
}

__attribute__ ((constructor))

void ctor() {
    int sigs[] = {         SIGILL, SIGFPE, SIGABRT, SIGBUS,
        SIGSEGV, SIGHUP, SIGINT, SIGQUIT,         SIGTERM     };
    int i;    
    struct sigaction sa;    
    sa.sa_handler = sighandler;    
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESETHAND;    
    for(i = 0; i < sizeof(sigs)/sizeof(sigs[0]); ++i) {
        if (sigaction(sigs[i], &sa, NULL) == -1) {
            perror("Could not set signal handler");        
        }    
    }
}
