#include <iostream>
#include "nheader.h"

using namespace std;

void fov(Map *map, int y, int x, int d)
{
  int visline[d*2+1];
  int visquare[d][d];
  int xdir;
  int ydir;
  int xspread;
  int yspread;
  int checkx;
  int checky;
  int numsquares;
  int vissum;
  int numchecked;
  int i,j,k,l;

  for (i=0;i<4;i++) {
    switch(i)
      {
      case 0:
	xdir=1;
	ydir=0;
	break;
      case 1:
	xdir=-1;
	ydir=0;
	break;
      case 2:
	xdir=0;
	ydir=1;
	break;
      case 3:
	xdir=0;
	ydir=-1;
	break;
      }
    xspread = -ydir;
    yspread = -xdir;
    for (j=0;j<2*d+1;j++) {visline[j]=d;}
    for (j=0;j<d;j++) {
      
      numsquares = 1 + 2*j;
      for (k=-j;k<j+1;k++) {
	vissum = 0;
	numchecked = 0;
	checkx = x + xdir * j + xspread * k;
	checky = y + ydir * j + yspread * k;

	for (l=((k+j)*(2*d+1))/(numsquares);l<((k+j+1)*(2*d+1))/(numsquares)+1;l++) {
	  vissum=visline[l]+vissum;
	  numchecked++;
	}
	if (vissum > (d * numchecked)/2) {
	  map->see(checky,checkx);
	}
	else {
	  for (l=((k+j)*(2*d+1))/(numsquares);l<((k+j+1)*(2*d+1))/(numsquares)+1;l++) {
	    visline[l]=0;
	  } 
	}
	
      }

      for (k=-j;k<j+1;k++) {
	checkx = x + xdir * j + xspread * k;
	checky = y + ydir * j + yspread * k;
	if (map->seethrough(checky,checkx) == 0) {
	  for (l=((k+j)*(2*d+1))/(numsquares);l<((k+j+1)*(2*d+1))/(numsquares)+1;l++) {
	    visline[l]=0;
	  }
	}
      }

    }
  }
}
