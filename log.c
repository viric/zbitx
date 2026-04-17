#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include "log.h"

void log_timed(const char *str)
{
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);

  static struct timespec last;
  struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	double last_s = last.tv_sec + last.tv_nsec / 1000000000.;
	double now_s = now.tv_sec + now.tv_nsec / 1000000000.;

	double diff_ms = (now_s - last_s) * 1000.;

	printf("[%.3f + %.1f ms] %s\n", now_s, diff_ms, str);
	fflush(stdout);

  pthread_mutex_unlock(&mutex);

	last = now;
}
