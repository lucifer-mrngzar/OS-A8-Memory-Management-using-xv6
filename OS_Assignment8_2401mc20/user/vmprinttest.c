//Ranveer Gupta 
//2401MC20
//ASSIGNMENT 8 Q3



#include "types.h"
#include "stat.h"
#include "user.h"

#define PG 4096

int
main(int argc, char *argv[])
{
  int touch = (argc > 1);
  char *heap;
  int i;

  printf(1, "\n######## 1. at start-up ########\n");
  vmprint();

  heap = sbrk(8 * PG);
  if(touch)
    for(i = 0; i < 8; i++)
      heap[i * PG] = i + 1;
  printf(1, "\n######## 2. after sbrk(8 * 4096)%s ########\n",
         touch ? " and touching the 8 pages" : "");
  vmprint();

  sbrk(-PG);
  printf(1, "\n######## 3. after sbrk(-4096) ########\n");
  vmprint();
  exit();
}
