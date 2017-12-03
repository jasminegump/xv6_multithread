#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct lock_t {
  uint state;      // state can be has lock (1) or must_wait (0)
  uint last_queue; // place after the one with the lock
  uint locked;
};

typedef struct  {
  uint state;      // state can be has lock (1) or must_wait (0)
  uint last_queue; // place after the one with the lock
} lock_t1;

uint pass_num = 0;
uint token = 0;
uint num_thread = 4;
struct lock_t lock;
uint total_pass = 40;

struct lock_t1 *array_lock;

uint *lock_array;
uint queue_last;


void
lock_b_initlock(struct lock_t *lk, uint pass)
{
  lock_array = (uint *)malloc(sizeof(uint)*pass);

  //lock_t1* array_lock= malloc(sizeof(lock_t1)*pass);
 // array_lock = malloc(sizeof(struct lock_t)*pass); // first initialize with 0's to make sure no garbages
  

  memset(lock_array, 0, sizeof(uint)*pass);
  //array_lock[0].state = 1;
  for(int i = 0; i < pass; i++)
  {
    if(i == 0) // first item will have state lock (1)
    {
      lock_array[i] = 1;
      //array_lock[i].last_queue = 
    }
    else // rest must_wait (0)
    {
      lock_array[i] = 0;
    }
  }
  queue_last = 0;

  lk->locked = 0;
}

void
lock_b_acquire(struct lock_t *lk)
{
  //while (thread_id != token);
  // The xchg is atomic.
  while(lock_array[queue_last] != 1);

  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
}

// Release the lock.
void
lock_b_release(struct lock_t *lk)
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
    lock_b_acquire(&lock);
    temp_thread_id = token;
    lock_b_release(&lock);
  } while((temp_thread_id != thread_id) && (pass_num < total_pass));

  // write
  if(pass_num < total_pass)
  {
    lock_b_acquire(&lock);

    lock_array[queue_last] = 0;
    if(queue_last >= num_thread)
    {
      queue_last = 0;
    }
    else
    {
      queue_last = queue_last + 1;
    }

    lock_array[queue_last] = 1;

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
    lock_b_release(&lock);
  }

  return 0;

}

int
main(int argc, char *argv[])
{
  
  int thread_id[num_thread];

  token = 0;

  lock_b_initlock(&lock, total_pass);

  // arg to pass in = thread ID
  for (int i = 0; i < num_thread; i++)
  {
    thread_id[i] = i;
    thread_create(&frisbee_game, &thread_id[i]);
  }

  wait();
  printf(1, "I'm finished.\n");
  exit();
}
