/*
 * -- Initialiser la bibliothèque avec timing_init();
 * -- Mémoriser un timestamp :
 *  tick_t t;
 *  GET_TICK(t);
 * -- Calculer un intervalle en microsecondes :
 *  TIMING_DELAY(t1, t2);
 */

#include "timing.h"

#ifndef min
#define min(a,b) \
	({__typeof__ ((a)) _a = (a); \
	  __typeof__ ((b)) _b = (b); \
	  _a < _b ? _a : _b; })
#endif

double scale_time = 0.0;
unsigned long long residual = 0;

void timing_init(void)
{
  static tick_t t1, t2;
  int i;
      
  residual = (unsigned long long)1 << 63;
  
  for(i = 0; i < 20; i++)
    {
      GET_TICK(t1);
      GET_TICK(t2);
      residual = min(residual, TICK_RAW_DIFF(t1, t2));
    }
  
  {
    struct timeval tv1,tv2;
    
    GET_TICK(t1);
    gettimeofday(&tv1,0);
    usleep(500000);
    GET_TICK(t2);
    gettimeofday(&tv2,0);
    scale_time = ((tv2.tv_sec*1e6 + tv2.tv_usec) -
	     (tv1.tv_sec*1e6 + tv1.tv_usec)) / 
      (double)(TICK_DIFF(t1, t2));
  }
}

double tick2usec(long long t)
{
  return (double)(t)*scale_time;
}
