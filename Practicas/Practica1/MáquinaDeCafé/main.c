#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "fsm.h"



#define COFFEE_PRICE    60
#define CUP_TIME	2500
#define COFFEE_TIME	3000
#define MILK_TIME	3000



enum cofm_state {

  COFM_WAITING,
  COFM_CUP,
  COFM_COFFEE,
  COFM_MILK,

};

enum purse_state{
  PURSE_WAITING,
};


static int coin = 0;
static int coin_insert (fsm_t* this) { if (coin > 0) return 1; else return 0; }
static int money = 0;

static int money_button = 0;
static int money_button_pressed (fsm_t* this) { return money_button; }

static int button = 0;
static int button_pressed (fsm_t* this) {if (money>=COFFEE_PRICE) return button; else return 0; }



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
  
static void cup (fsm_t* this)

{
  money-=COFFEE_PRICE;
  printf("Poniendo taza...\n");
  timer=0;
  timer_start(CUP_TIME);
  button = 0;

}



static void coffee (fsm_t* this)

{
  printf("Echando café...\n");
  timer=0;
  timer_start(COFFEE_TIME);
}



static void milk (fsm_t* this)

{
  printf("Echando leche...\n");
  timer=0;
  timer_start(MILK_TIME);
}



static void finish (fsm_t* this)

{
  printf("Led de preparado a 1\n");
  if (money>0) money_button=1;
}


static void money_back (fsm_t* this) {
  printf("Se devuelve un total de %d \n", money);
  money = 0;
  money_button = 0;
}


static void money_add (fsm_t* this) {
  money+= coin;
  coin=0;
}


// Explicit FSM description

static fsm_trans_t cofm[] = {

  { COFM_WAITING, button_pressed, COFM_CUP,     cup    },
  { COFM_CUP,     timer_finished, COFM_COFFEE,  coffee },
  { COFM_COFFEE,  timer_finished, COFM_MILK,    milk   },
  { COFM_MILK,    timer_finished, COFM_WAITING, finish },
  {-1, NULL, -1, NULL },

};

static fsm_trans_t purse[] = {

  { PURSE_WAITING, coin_insert, PURSE_WAITING, money_add},
  { PURSE_WAITING, money_button_pressed, PURSE_WAITING, money_back},
  {-1, NULL, -1, NULL },

};



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


  fsm_t* cofm_fsm = fsm_new (cofm);
  fsm_t* purse_fsm = fsm_new (purse);


  gettimeofday (&next_activation, NULL);
  
  while (scanf("%d %d %d", &button, &coin, &money_button)==3) {
       
      fsm_fire (cofm_fsm);
      fsm_fire (purse_fsm);
      gettimeofday (&next_activation, NULL);
      timeval_add (&next_activation, &next_activation, &clk_period);
      delay_until (&next_activation);
          
  }
  return 1;
}
