#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

// define some global lock and pass
// lock

// lock init
//lock acquire
//lock release

struct lock_t {
  uint locked;       // Is the lock held?
};

uint pass_num = 0;
uint token = 0;
int num_thread = 4;
struct lock_t lock;
uint total_pass = 6;
/*
typedef struct 
{
	uint field1;
	lock_t field2;sx
} arguments;

typedef struct 
{
	uint pass;
	uint thread_id;
} critical_data;
*/

void
lock_a_initlock(struct lock_t *lk)
{
  lk->locked = 0;
}

void
lock_a_acquire(struct lock_t *lk, int thread_id)
{
  while (thread_id != token);
  // The xchg is atomic.
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
}

// Release the lock.
void
lock_a_release(struct lock_t *lk)
{
  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

  // Release the lock, equivalent to lk->locked = 0.
  // This code can't use a C assignment, since it might
  // not be atomic. A real OS would use C atomics here.
  asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

void
thread_create(void *(*start_routine)(void *), void *arg)
{
  int pid;
  char *sp;
  //char i;

  sp = (char *)malloc(4096); // Page size 
  memset(sp, 0, 4096);
  //printf(1, "stack location: %x\n", sp);
  //printf(1, "Clone test\n");

  pid = clone(sp, 4096);
  if(pid == 0)
  {
  	//while(1);
  	//printf(1, "Created child!\n");
  	//printf(1, "PID: %d\n", pid);
    start_routine(arg); 
  	while(1);
  }
  else
  {
  	//start_routine(arg);
  	//while(1);
	//printf(1, "Parent!\n");
	//printf(1, "PID: %d\n", pid);
	//start_routine(arg);
  }
}


void*
do_nothing(void* arg)
{

  int thread_id = *(int*)arg;
	//while(1);
  lock_a_acquire(&lock, thread_id);
  
	printf(1, "Thread_ID: %d\n", thread_id );
  printf(1, "Pass Number: %d\n", pass_num );

  pass_num = pass_num + 1;
  if (thread_id == (num_thread - 1))
    {
      token = 0;
    }
  else 
    {
      token = token + 1;
    }
  lock_a_release(&lock);

	//while(1);
	return 0;
}



// start routine(arg)  
// acquire_lock ()
// modify global
// release lock
// return 0


int
main(void)
{
  //int x = 4;
  int thread_id[num_thread];
  //int total_pass = 6;
  token = 0;

  //initialize lock;

 // lock.locked = 0;
  lock_a_initlock(&lock);


  //lock_t lock = 0;

  // initalize lock
  //lock_t lock = 0;

  // initialize global (thread number, pass number, , lock that was just initialized)
  

  // arg to pass in = thread ID
  for (int i = 0; i < num_thread; i++)
  {
    thread_id[i] = i;
    thread_create(&do_nothing, &thread_id[i]);
  }

  while(1);
  exit();
}







