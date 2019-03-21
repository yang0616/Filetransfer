#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

//  thread start
void thread_start(int c);

//work thread
void*  work_thread(void *arg);

