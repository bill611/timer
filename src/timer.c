/*
 * =============================================================================
 *
 *       Filename:  Timer.c
 *
 *    Description:  定时器
 *
 *        Version:  v1.0
 *        Created:  2016-08-08 18:33:01
 *       Revision:  none
 *
 *         Author:  xubin
 *        Company:  Taichuan
 *
 * =============================================================================
 */
/* ---------------------------------------------------------------------------*
 *                      include head files
 *----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "timer.h"
/* ---------------------------------------------------------------------------*
 *                  extern variables declare
 *----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*
 *                  internal functions declare
 *----------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------*
 *                        macro define
 *----------------------------------------------------------------------------*/

typedef struct _TimerPriv {
	timer_t real_id;

	int enable;
	unsigned int speed;
	unsigned int count;
	unsigned int count_old;

	void (*func)(void);
}TimerPriv;
/* ---------------------------------------------------------------------------*
 *                      variables define
 *----------------------------------------------------------------------------*/

static void timerStart(Timer *This)
{
	if (This->priv->enable) {
		printf("Timer already start\n");
		return;
	}
	This->priv->count_old = This->priv->count = This->getSystemTick();
	This->priv->enable = 1;
}

static void timerStop(Timer *This)
{
	if (This->priv->enable == 0) {
		printf("Timer stopped\n");
		return;
	}
	This->priv->enable = 0;
}

static void timerDestroy(Timer *This)
{
	if (This->priv->real_id)
		This->realTimerDelete(This);
	if (This->priv)
		free(This->priv);
	if (This)
		free(This);
}

static int timerHandle(Timer *This)
{
	if (This->priv->enable == 0) {
		return 0;
	}
	int ret = 0;
	if ((This->priv->count - This->priv->count_old) >= This->priv->speed) {
		if (This->priv->func) {
			This->priv->func();
			ret = 1;
		}
		This->priv->count_old = This->priv->count;
		// This->priv->count = 0;
	} else {
		// This->priv->count++;
		This->priv->count = This->getSystemTick();
	}
	return ret;
}

static unsigned int getSystemTickDefault(void)
{
	printf("[%s]\n", __FUNCTION__);
	return 0;
}

static void realTimerCreateDefault(Timer *This,double value,void (*function)(int timerid,int arg))
{
	printf("[%s]\n", __FUNCTION__);
	struct sigevent evp;
	struct itimerspec ts;
	int ret;
	evp.sigev_value.sival_ptr = &This->priv->real_id;
	evp.sigev_notify = SIGEV_SIGNAL;
	evp.sigev_signo = SIGUSR2;
	signal(SIGUSR2, (__sighandler_t)function);
	ret = timer_create(CLOCK_REALTIME, &evp, &This->priv->real_id);
	if(ret)
		perror("timer_create");
	ts.it_interval.tv_sec = (int)value;
	ts.it_interval.tv_nsec = (value - (int)value)*1000000;
	ts.it_value.tv_sec = (int)value;
	ts.it_value.tv_nsec = (value - (int)value)*1000000;
	ret = timer_settime(This->priv->real_id, 0, &ts, NULL);
	if(ret)
		perror("timer_settime");
	// timer_create (CLOCK_REALTIME, NULL, &This->priv->real_id);
	// timer_connect (This->priv->real_id, function,0);
	// timer_settime (This->priv->real_id, 0, value, NULL);
}

static void realTimerDeleteDefault(Timer *This)
{
	timer_delete(This->priv->real_id);
	This->priv->real_id = 0;
}

Timer * timerCreate(int speed,void (*function)(void))
{
	Timer *This = (Timer *) calloc(1,sizeof(Timer));
	if (!This) {
		printf("timer alloc fail\n");
		return NULL;
	}
	This->priv = (TimerPriv *) calloc(1,sizeof(TimerPriv));
	if (!This->priv){
		printf("timer alloc fail\n");
		free(This);
		return NULL;
	}
	This->priv->speed = speed;
	This->priv->func = function;

	This->start = timerStart;
	This->stop = timerStop;
	This->destroy = timerDestroy;
	This->handle = timerHandle;
	This->getSystemTick = getSystemTickDefault;

	This->realTimerCreate = realTimerCreateDefault;
	This->realTimerDelete = realTimerDeleteDefault;
	return This;
}
