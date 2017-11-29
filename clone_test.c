#include "types.h"
#include "stat.h"
#include "user.h"
/*
void
clonetest(void)
{
  int pid;
  char *sp;
  char i;

  sp = (char *)malloc(4096); // Page size 
  memset(sp, 0, 4096);
  printf(1, "stack location: %x\n", sp);
  printf(1, "Clone test\n");

  i = 'A';
  sp = &i;

  pid = clone(sp, 4096);
  if(pid == 0)
  {
  	printf(1, "Created child!\n");
  	printf(1, "PID: %d\n", pid);
  	while(1);
  }
  else
  {
	printf(1, "Created parent!\n");
	printf(1, "PID: %d\n", pid);
  }

}
*/
void
thread_create(void *(*start_routine)(void *), void *arg)
{
  int pid;
  char *sp;
  //char i;

  sp = (char *)malloc(4096); // Page size 
  memset(sp, 0, 4096);
  printf(1, "stack location: %x\n", sp);
  printf(1, "Clone test\n");

  pid = clone(sp, 4096);
  if(pid == 0)
  {
  	start_routine(arg);
  	//while(1);
  	//printf(1, "Created child!\n");
  	//printf(1, "PID: %d\n", pid);
  	//while(1);
  }
  else
  {
  	//start_routine(arg);
  	//while(1);
	//printf(1, "Created parent!\n");
	//printf(1, "PID: %d\n", pid);
	//start_routine(arg);
  }
}

void*
do_nothing(void* arg)
{
	int num = *(int*)arg;
	//while(1);
	printf(1, "Got: %d", num);
	//while(1);
	return 0;
}

int
main(void)
{
  int x = 4;
  x = 4;
  thread_create(&do_nothing, &x);
  //x = 5;
  //thread_create(&do_nothing, &x);
  //x = 6;
  //thread_create(&do_nothing, &x);
  while(1);
  exit();
}







