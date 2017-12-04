#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

struct lock_t {
  uint locked;
};

// The following are global variables
uint pass_num = 0;
uint token = 0;
uint num_thread = 100;
uint total_pass = 100;
struct lock_t lock; // This is the global lock that all threads utilize
uint seq_number; // This is the global variable used to determine whether a thread can read or write

// SeqLock initialization
// Initialize seq_number which is used to determine whether a thread can read or write
// Also initialize lock
void
lock_c_initlock(struct lock_t *lk)
{
  seq_number = 0;
  lk->locked = 0;
}

// SeqLock acquire
// When acquire is called (only for writes), it merely checks to see if the global lock is available
// And increments seq_number
void
lock_c_acquire(struct lock_t *lk)
{
  
  while(xchg(&lk->locked, 1) != 0)
    ;

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();
  seq_number = seq_number + 1;
}

// Release the lock.
// When release is called (only for writes), it merely releases the global lock
// And increments seq_number again (so next thread can decide to write)
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

void*
frisbee_game(void* arg)
{

  int thread_id = *(int*)arg;
  uint temp_seq;

  // The following is a read of the token
  // In sequential lock, you don't need to use a lock if you're just reading 
  // So just do a read without lock to see if token matches the thread ID
  // You know you can write if sequential number (seq_number) is EVEN and your token matches your thread ID
  // If it does not match, continue looping until it's its turn
  // If it does match, exit this while loop to continue on 
  
  do
  {
    do
    {
      temp_seq = seq_number;
    } while((seq_number != temp_seq) && (seq_number % 2 == 1)); // Keep trying until Seq number is EVEN and local seq = global lock seq
  }
  while((token != thread_id) && (pass_num < total_pass));

  // The following is the write of the token
  // In sequential lock, you need to acquire lock at this time because you only acquire the lock during a WRITE
  // Lock is still held at this time
  // So update token value to next thread
  // Update Pass as well and then release lock for next thread to access
  if(pass_num < total_pass)
  {
    lock_c_acquire(&lock);
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
    lock_c_release(&lock);
  }
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
  printf(1,"This is seqlock\n");

  arg1 = (atoi(argv[1]));
  arg2 = (atoi(argv[2]));

  num_thread = (uint)arg1;
  total_pass = (uint)arg2;

  // Initialize token to 0
  token = 0;

  // Make a call to start tick count on the number of times scheduler scheduled
  time_0 = starttime();

  // Initialize sequential lock 
  lock_c_initlock(&lock);

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







