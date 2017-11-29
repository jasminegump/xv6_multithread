// #Asanka:
// A simple program which just print something on screen

#include "types.h"
#include "stat.h"
#include "user.h"

int get_a_date(int year, int day)
{
	int x, y;
	x = year;
	y = day;
	return (x + y);
}

int
main(void)
{
 int x;
 x = get_a_date(2017, 1);
 printf(1, "%d", x);
 //getyear(2017);
 //printf(1, "This year is %d\n", getyear(2017));
 exit();
}

