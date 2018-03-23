#include <iostream>
#include "nheader.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

enum openclosed {
  opened,
  closed,
  empty
};

typedef struct tilenode {
  int x;
  int y;
  int px;
  int py;
  int F;
  int G;
  int H;
  openclosed state;
} tnode;

tnode createtnode(int y, int x, int ty, int tx, tnode * parent);

int calch(int y, int x, int ty, int tx);

void sortopen(tnode * openlist[], int numopen)
{
  // Bubblesort. Put big F values on the bottom.
  int i;
  tnode * temp;
  int repeat=1;
  while (repeat==1) {
    repeat=0;
    for (i=0;i<numopen-1;i++) {
      if ((*(openlist[i])).F<(*(openlist[i+1])).F) {
	temp = openlist[i];
	openlist[i]=openlist[i+1];
	openlist[i+1]=temp;
	repeat=1;
      }
    }
  }
}

// find a path!
llist * findapath(int sy, int sx, int ty, int tx, Map *map, llist ** moves, Screen * vscreen)
{
  if (map->cango(ty,tx)==0) {return NULL;}
  if (sy==ty && sx==tx) {return NULL;}
  int i,j,k;
  int x,y;
  int sizex=map->xsize;
  int sizey=map->ysize;
  int size=sizex*sizey;
  tnode nodes[sizey][sizex];
  tnode newtnode;
  tnode * pwt;
  int numopen=0;
  tnode * openlist[size];

  *moves=NULL;

  newtnode.x=0; newtnode.y=0; newtnode.F=0; newtnode.H=0; newtnode.state=empty; newtnode.px=-2; newtnode.py=-1;

  for (i=0;i<sizex;i++) {
    for (j=0;j<sizey;j++) {
      nodes[j][i]=newtnode;
      openlist[i+sizex*j]=NULL;
    }
  }

  x=sx;
  y=sy;
  
  numopen++;
  newtnode = createtnode(y,x,ty,tx,NULL);
  nodes[y][x]=newtnode;
  pwt=&(nodes[y][x]);

  openlist[numopen-1]=pwt;
  pwt=NULL;
  nodes[y][x].px=-1;
  nodes[y][x].py=-1;
  int numpoints=0;
  while ((x!=tx || y!=ty) && (numopen > 0)) {
    numpoints++;
    pwt=openlist[numopen-1];
    openlist[numopen-1]=NULL;
    numopen--;

    (*pwt).state=closed;
    x=(*pwt).x;
    y=(*pwt).y;

    if (y==ty && x==tx) {break;}
    for (i=x-1;i<x+2;i++) {
      for (j=y-1;j<y+2;j++) {

	if (map->cango(j,i) == 1) {
	  if (nodes[j][i].state != closed) {
	    newtnode = createtnode(j,i,ty,tx,pwt);
	    if (map->checkmonster(j,i) != NULL) {
	      if ((*(map->checkmonster(j,i))).isplayer != 1) {
		newtnode.G=10*newtnode.G;
		newtnode.F=newtnode.H+newtnode.G;
	      }
	    }
	    if (nodes[j][i].state != opened) {

	      nodes[j][i] = newtnode;
	      numopen++;
	      openlist[numopen-1]=&(nodes[j][i]);
	      sortopen(openlist,numopen);
	      nodes[j][i].px=x;
	      nodes[j][i].py=y;

	    }
	    else if (newtnode.G < nodes[j][i].G) {

	      nodes[j][i]=newtnode;
	      sortopen(openlist,numopen);
	      nodes[j][i].px=x;
	      nodes[j][i].py=y;
	    }
	  }
	}
      }
    }
  }

  if (x != tx && y != ty) {return NULL;}

  // Construct route;
  x=tx;
  y=ty;
  /*  pwt = &(nodes[y][x]);
  x=(*pwt).x;
  y=(*pwt).y;*/
  coords * direction;

  int px,py;
  int maxiters=50;
  
  while ( x != sx || y != sy ) {
    maxiters--;
    //    if (maxiters<0) {return NULL;}
    px = nodes[y][x].px;
    py = nodes[y][x].py;
    //    pwt = &(nodes[py][px];
    if (px == -1 || py == -1) {
      char buf[5];
      sprintf(buf, "%d", numpoints);
      vscreen->message(buf);
      break;
    }
    //    numpoints++;
    direction = (coords*)malloc(sizeof(coords));
    (*direction).x=x;
    (*direction).y=y;
    list_add(moves,(void*)direction);
    x = px;
    y = py;
    direction=NULL;

  }

  return *moves;
}

//char calcdir(int sy, int sx, int ty, int tx)

tnode createtnode(int y, int x, int ty, int tx, tnode * parent)
{
  tnode newtnode;
  newtnode.x=x;
  newtnode.y=y;
  newtnode.state=opened;
  newtnode.px=-1;
  newtnode.py=-1;
  if (parent != NULL) {
    newtnode.px=(*parent).x;
    newtnode.py=(*parent).y;
    if (newtnode.px - (*parent).x != 0 && newtnode.py - (*parent).y != 0) {
      newtnode.G=(*parent).G+14;
    }
    else {
      newtnode.G=(*parent).G+10;
    }
  }
  else {newtnode.G=0;}
  newtnode.H=calch(y,x,ty,tx);
  newtnode.F=newtnode.G + newtnode.H;

  return newtnode;
}

int calch(int y, int x, int ty, int tx)
{
  int difx, dify, difdif, cost;
  difx = abs(tx-x);
  dify = abs(ty-y);
  difdif = abs(difx-dify);
  cost=10*difdif;
  if (dify > difx) {cost=cost+14*(dify-difdif);}
  else {cost=cost+14*(difx-difdif);}

  return cost;
}
