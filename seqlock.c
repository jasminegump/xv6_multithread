#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct lock_t {
  uint state;      // state can be has lock (1) or must_wait (0)
  uint last_queue; // place after the one with the lock
  uint locked;
};

uint pass_num = 0;
uint token = 0;
uint num_thread = 20;
struct lock_t lock;
uint total_pass = 40;
uint seq_number;

void
lock_c_initlock(struct lock_t *lk)
{
  seq_number = 0;
  lk->locked = 0;
}

void
lock_c_acquire(struct lock_t *lk)
{
  //while (thread_id != token);
  // The xchg is atomic.
  
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
  seq_number = seq_number + 1;
}

// Release the lock.
void
lock_c_release(struct lock_t *lk)
{
  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  seq_number = seq_number + 1;
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
      start_routine(arg); 
    }

    free(sp);
    exit();
  }
}

void*
frisbee_game(void* arg)
{

  int thread_id = *(int*)arg;
  uint temp_seq;



    // reads
    do
    {
      do
      {
        temp_seq = seq_number;
      } while((seq_number != temp_seq) && (seq_number % 2 == 1)); // Keep trying until Seq number is EVEN and local seq = global lock seq
    }
    while((token != thread_id) && (pass_num < total_pass));

  if(pass_num < total_pass)
  {
    // write
    lock_c_acquire(&lock);
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
    lock_c_release(&lock);
  }
  return 0;

}

int
main(int argc, char *argv[])
{
  
  int thread_id[num_thread];

  token = 0;

  lock_c_initlock(&lock);

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







