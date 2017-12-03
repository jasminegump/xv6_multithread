#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct lock_t {
  uint locked;       // Is the lock held?
};

uint pass_num = 0;
uint token = 0;
uint num_thread = 20;
struct lock_t lock;
uint total_pass = 40;

void
lock_a_initlock(struct lock_t *lk)
{
  lk->locked = 0;
}

void
lock_a_acquire(struct lock_t *lk)
{
  //while (thread_id != token);
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
  //int* ret_pass;
  //int pass_val = 0;
  //int thread_id = *(int*)arg;
  //char i;

  sp = (char *)malloc(4096); // Page size 
  memset(sp, 0, 4096);
  //printf(1, "stack location: %x\n", sp);
  //printf(1, "Clone test\n");

  pid = clone(sp, 4096);
  if(pid == 0)
  {
  	//printf(1, "Created child!\n");
  	//printf(1, "PID: %d\n", pid);
    while (pass_num < (total_pass))
    {
      //if (token == thread_id)
      //{
      start_routine(arg); 
      //}

    }

    free(sp);
    exit();
  }
}

void*
frisbee_game(void* arg)
{

  int thread_id = *(int*)arg;
  uint temp_thread_id;

  // read
  do
  {
    lock_a_acquire(&lock);
    temp_thread_id = token;
    lock_a_release(&lock);
  } while((temp_thread_id != thread_id) && (pass_num < total_pass));

  // write
  if(pass_num < total_pass)
  {
    lock_a_acquire(&lock);
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
    printf(1, "thread %d is passing token to thread %d\n", thread_id, token);
    lock_a_release(&lock);
  }

  return 0;

}

int
main(int argc, char *argv[])
{
  
  int thread_id[num_thread];

  // couldn't get this quite working for numbers above 10 so it's commented out.
  /*int arg1;
  int arg2;

  arg1 = (atoi(argv[1]));
  arg2 = (atoi(argv[2]));
  num_thread = (uint)arg1;
  total_pass = (uint)arg2;

// get it working with anything greater than 9
  if (argc != 3)
  {
    printf(1, "You need at least 3 arguments.\n");
    //return 0;
  }

  //printf(1, "arg1 %d\n", atoi(argv[1]));
  //printf(1, "arg2 %d\n", atoi(argv[2]));
*/

  token = 0;

  lock_a_initlock(&lock);

  // arg to pass in = thread ID
  for (int i = 0; i < num_thread; i++)
  {
    thread_id[i] = i;
    thread_create(&frisbee_game, &thread_id[i]);
  }

  for (int i = 0; i < num_thread; i++)
  {
    wait(); 
  }
  printf(1, "I'm finished.\n");
  exit();
}







