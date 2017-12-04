#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct lock_t {
  uint locked;
};

uint pass_num = 0;
uint token = 0;
uint num_thread = 100;
uint total_pass = 100;
struct lock_t lock; // This is the global lock that all threads must access

// Spin lock initialization
// Must be open for first thread to access
void
lock_a_initlock(struct lock_t *lk)
{
  lk->locked = 0;
}

// Spin lock acquire
// Wait until the global locked is available
// Got this from Spin Lock
void
lock_a_acquire(struct lock_t *lk)
{
// Uses xchg atomic operation to update locked
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
}

// Release the lock.
// At this point, thread is ready to release its rights
// So it sets locked to 0 for other threads toa ccess it
// Using an atomic operation
// Got this from spin lock
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

// This function creates the threads and has the child process
// call the frisbee function
void
thread_create(void *(*start_routine)(void *), void *arg)
{
  int pid;
  char *sp;
  // Allocate space for the stack to pass it to clone
  sp = (char *)malloc(4096); // Page size 
  memset(sp, 0, 4096);
  pid = clone(sp, 4096);
  
  // If process returned is a child (PID = 0), while you haven't hit the pass
  // call frisbee
  if(pid == 0)
  {
    while (pass_num < (total_pass))
    {
      start_routine(arg); 

    }
	// Once child process is finished passing, free up the stack space
	// and exit
    free(sp);
    exit();
  }
}

// The following is the frisbee test game
void*
frisbee_game(void* arg)
{
  // Read in arguments passed in main function, which in this case is thread number
  int thread_id = *(int*)arg;
  uint temp_token;

  // The following is a read of the token
  // Acquire lock to see if token matches the thread ID
  // If it does not match, release and continue looping until it's its turn
  // If it does match, exit this while loop to continue on without releasing the lock
  do
  {
    lock_a_acquire(&lock);
    temp_token = token;
    if(temp_token != thread_id)
    {
      lock_a_release(&lock);
    }
  } while((temp_token != thread_id) && (pass_num < total_pass));

  // The following is the write of the token
  // Lock is still held at this time
  // So update token value to next thread
  // Update Pass as well and then release lock for next thread to access
  if(pass_num < total_pass)
  {
    pass_num = pass_num + 1;
    if (thread_id == (num_thread - 1))
      {
        token = 0;
      }
    else 
      {
        token = token + 1;
      }
    printf(1, "Pass Number no: %d, Thread %d is passing token to thread %d\n", pass_num, thread_id, token);  
  }
  lock_a_release(&lock);
  

  return 0;

}

int
main(int argc, char *argv[])
{
  int thread_id[num_thread];
  int arg1;
  int arg2;
  int time_0;
  int time_1;

  // The following is to read in user input
  if (argc != 3)
  {
    printf(1, "You need at least 3 arguments.\n");
  }
  printf(1,"This is spinlock\n");

  arg1 = (atoi(argv[1]));
  arg2 = (atoi(argv[2]));

  num_thread = (uint)arg1;
  total_pass = (uint)arg2;

  // Initialize token to 0
  token = 0;

  // Make a call to start tick count on the number of times scheduler scheduled
  time_0 = starttime();

  // Initialize array lock
  lock_a_initlock(&lock);

  // Create num_threads 
  for (int i = 0; i < num_thread; i++)
  {
    thread_id[i] = i;
    thread_create(&frisbee_game, &thread_id[i]);
  }

  // Wait for all child processes to finish
  // Which in this case is all the threads that were created
  for (int i = 0; i < num_thread; i++)
  {
    wait(); 
  }
  
  // End tick count
  time_1 = endtime();
  time_1 = time_1 - time_0;
  printf(1, "Simulation of Frisbee game has finished!\nNumber of times the scheduler switched process: %d.\n", time_1);
  
  // Parent process can now terminate
  exit();
}







