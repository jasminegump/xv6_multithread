#include "types.h"
#include "stat.h"
#include "user.h"

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

int
main(void)
{
  clonetest();
  clonetest();
  while(1);
  exit();
}