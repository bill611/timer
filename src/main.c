#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "timer.h"

Timer *timer;

static unsigned int getSystemTick(void)
{
	struct  timeval tv;                            
    gettimeofday(&tv,NULL);                           
    return ((tv.tv_usec / 1000) + tv.tv_sec  * 1000 );
}

static void timer_func(void)
{
	static int count = 0;
	printf("[%s]%d\n", __FUNCTION__,count++);	
}

static void timer_real_func(int timerid,int arg)
{
	timer->handle(timer);
}

int main(int argc, char *argv[])
{
	timer = timerCreate(LAYER_TIME_1S,timer_func);
	timer->getSystemTick = getSystemTick;
	timer->realTimerCreate(timer,0.01,timer_real_func);
	timer->start(timer);
	while (1) {
		sleep(3);
	}
	timer->destroy(timer);
	return 0;
}
