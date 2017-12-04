#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

// Use a lock struct only to keep track of my_place as required by the array locking mechanism
struct lock_t {
  uint my_place;
};

// The following are global variables
uint pass_num = 0;
uint token = 0;
uint num_thread = 100;
uint total_pass = 100;
uint *lock_array; // This is the global lock array that all threads utilize
uint queue_last; // This is the global queue last that can be only modified by one thread at a time

// Array lock initialization
void
lock_b_initlock(uint number_of_threads)
{
  // Initialize an array of size number of threads
  lock_array = (uint *)malloc(sizeof(uint)*number_of_threads);
  // Initialize array with 0's
  memset(lock_array, 0, sizeof(uint)*number_of_threads);
  
  // Initialize lock array
  for(int i = 0; i < number_of_threads; i++)
  {
    if(i == 0) // first index in array will have lock (1)
    {
      lock_array[i] = 1;
    }
    else // All other indicies in array will be must_wait (0)
    {
      lock_array[i] = 0;
    }
  }
  // Initializ queue_last to start at 0
  queue_last = 0;

}

// Array lock acquire
// Fetch and increment logic
// A thread will wait for its turn by checking its index in lock array
// and move the queue_last along so other threads can get a chance.
// Setting queue_last is an atomic operation
void
lock_b_acquire(struct lock_t *lk)
{
  int my_place;

  // Fetch my place
  my_place = queue_last;

  // Update local lock struct with my index
  lk->my_place = my_place;

  // Increment queue_last using an atomic operation
  // Need to make sure it's a circular array
  if(queue_last >= num_thread -1)
  {
    xchg(&queue_last, 0);
  }
  else
  {
    xchg(&queue_last, queue_last + 1);
  }

  // Wait for my place
  while(lock_array[my_place] != 1);

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that the critical section's memory
  // references happen after the lock is acquired.
  __sync_synchronize();

}

// Release the lock.
// At this point, the current thread is ready to release its rights
// So it sets its index in the array lock to 0 and sets the next index in 
// array lock with a 1 such that the next thread can get a chance.
void
lock_b_release(struct lock_t *lk)
{
  lock_array[lk->my_place] = 0;
  if(lk->my_place >= num_thread - 1)
  {
    lock_array[0] = 1;
  }
  else
  {
    lock_array[lk->my_place + 1] = 1;
  }

  // Tell the C compiler and the processor to not move loads or stores
  // past this point, to ensure that all the stores in the critical
  // section are visible to other cores before the lock is released.
  // Both the C compiler and the hardware may re-order loads and
  // stores; __sync_synchronize() tells them both not to.
  __sync_synchronize();

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
  struct lock_t lock;
  int thread_id = *(int*)arg;
  uint temp_token;

  // The following is a read of the token
  // Acquire lock to see if token matches the thread ID
  // If it does not match, release and continue looping until it's its turn
  // If it does match, exit this while loop to continue on without releasing the lock
  do
  {
    lock_b_acquire(&lock);
    temp_token = token;
    if (temp_token != thread_id)
    {
      lock_b_release(&lock);
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
    lock_b_release(&lock);

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
  printf(1,"This is array lock\n");

  arg1 = (atoi(argv[1]));
  arg2 = (atoi(argv[2]));

  num_thread = (uint)arg1;
  total_pass = (uint)arg2;

  // Initialize token to 0
  token = 0;

  // Make a call to start tick count on the number of times scheduler scheduled
  time_0 = starttime();

  // Initialize array lock
  lock_b_initlock(num_thread);

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
