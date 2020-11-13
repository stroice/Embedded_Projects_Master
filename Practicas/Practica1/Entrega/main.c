#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "fsm.h"

#define Wait_Time 3000
#define top 2
#define middle 1
#define bottom 0

enum main_state {
  DOWN,
  OPENING,
  OPEN,
  WAITING_UP,
  CLOSING,
};

enum motor_control {
  NOP,
  RAISE,
  LOWER,
};

static enum motor_control motor = NOP;

static int gatePosition;
static int gatePosition_raise(fsm_t* this) { if (gatePosition == top) return 1; else return 0; }
static int gatePosition_lower(fsm_t* this) { if (gatePosition == bottom) return 1; else return 0; }


static int carAtGate;
static int carAtGate_active(fsm_t* this) { return carAtGate; }

static int carJustExit;
static int carJustExit_active(fsm_t* this) { return carJustExit; }

static int timer = 0;
static void timer_isr (union sigval arg) { timer = 1; }
static void timer_start (int ms)

{
  timer_t timerid;
  struct itimerspec spec;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  spec.it_value.tv_sec = ms / 1000;
  spec.it_value.tv_nsec = (ms % 1000) * 1000000;
  spec.it_interval.tv_sec = 0;
  spec.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &spec, NULL);
}
static int timer_finished (fsm_t* this) { return timer; }


static void abriendo (fsm_t* this)
{
  printf("Subiendo barrera\n");
  motor = RAISE;
  carJustExit = 0;
}

static void abierta (fsm_t* this)
{
  motor = NOP;
  printf("Barrera abierta\n");
}

static void esperando (fsm_t* this)
{
  motor = NOP;
  printf("Vehiculo ha salido, esperando 3 segundos\n");
  timer=0;
  timer_start (Wait_Time);
  carJustExit = 0;
}

static void cerrando (fsm_t* this)
{
  motor = LOWER;
  printf("Cerrando barrera\n");
}

static void cerrado(fsm_t* this)
{
  motor = NOP;
  printf("Barrera cerrada\n");
}
 



// Explicit FSM description
static fsm_trans_t gate[] = {
  { DOWN,         carAtGate_active,   OPENING,    abriendo  },
  { OPENING,      gatePosition_raise, OPEN,       abierta   },
  { OPEN,         carJustExit_active, WAITING_UP, esperando },
  { WAITING_UP,   timer_finished,     CLOSING,    cerrando  },
  { CLOSING,      gatePosition_lower, DOWN,       cerrado   },
  {-1, NULL, -1, NULL },
  };


// Utility functions, should be elsewhere

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000; 
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}


int main ()
{
  struct timeval clk_period = { 0, 250 * 1000 };
  struct timeval next_activation;

  fsm_t* gate_fsm = fsm_new(gate);

  gettimeofday(&next_activation, NULL);

  while (scanf("%d %d %d", &gatePosition, &carAtGate, &carJustExit)==3) {

    fsm_fire(gate_fsm);
    gettimeofday (&next_activation, NULL);
    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);

  }
  return 1;
}
