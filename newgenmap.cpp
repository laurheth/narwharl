// New and hopefully better map generator... Less cobbled together.

#include <iostream>
#include "nheader.h"
#include <cstring>
#include <fstream>
#include <cmath>

using namespace std;

extern Stuffgetter * objget;

#define PLANRES (10)

tile walltile;
tile floortile;
tile doortile;
tile holetile;

int lastroomy, lastroomx;

void placeitem(int y, int x, Map * map) {
  int c=(int)(mtrand()*100)+1;
  if (c <= 15) {
    map->placeweapon(y,x);
  }
  else if (c > 15 && c <= 30) {
    map->placescroll(y,x);
  }
  else if (c > 30 && c <= 35) {
    map->placewand(y,x);
  }
  else if (c > 35 && c <= 55) {
    map->placearmor(y,x);
  }
  else if (c > 55 && c <= 75) {
    map->placepotion(y,x);
  }
  else if (c > 75 && c <= 90) {
    map->placelauncher(y,x);
  }
  else {
    map->placeammo(y,x);
  }
}

int addroom(int ly, int lx, int hy, int hx, int py, int px, Map * map);
int addroom(int ly, int lx, int hy, int hx, int py, int px, Map * map, int mustberoom);

tile Map::gettile(int y, int x)
{
  return map[y][x];
}

void Map::settile(int y, int x, tile newtile)
{
  int i,j;
  if (this->inside(y,x) == 1) {
    // Move a creature at a drawn over tile to elsewhere
    if (map[y][x].creature != NULL) {
      Creature * movemon = map[y][x].creature;
      do {
	i=(int)(xsize*mtrand());
	j=(int)(ysize*mtrand());
      } while (	this->cango(j,i) == 0 || map[j][i].creature != NULL || map[j][i].symbol == '>' || map[j][i].symbol == '<' || (y==j && x==i));
      this->leave(y,x);
      this->go(movemon,j,i);
      movemon->x=i;
      movemon->y=j;
    }
    // Same with items
    if (map[y][x].ilist != NULL) {
      llist * movestuff = map[y][x].ilist;
      do {
	i=(int)(xsize*mtrand());
	j=(int)(ysize*mtrand());
      } while (	this->cango(j,i) == 0 || map[j][i].ilist != NULL || map[j][i].symbol == '>' || map[j][i].symbol == '<');
      map[j][i].ilist = movestuff;
      map[y][x].ilist = NULL;
    }
    map[y][x]=newtile;
  }
}

int checksurroundingtiles(int y, int x, Map *map)
{
  int i,j;
  int neighbours=0;
  for (i=x-1;i<x+2;i++) {
    for (j=y-1;j<y+2;j++) {
      if (j != y || i != x) {
	if (map->cango(j,i) == 0 || map->inside(j,i)==0) {
	  neighbours++;
	}
      }
    }
  }
  return neighbours;
}

int acceptabletodraw(int y, int x, Map *map)
{
  int i,j;
  for (i=x-1;i<x+2;i++) {
    for (j=y-1;j<y+2;j++) {
      if (j != y || i != x) {
	if (map->cango(j,i) == 1 || map->inside(j,i)==0 || map->canshoot(j,i) == 1) {
	  return 0;
	}
      }
    }
  }
  return 1;
}

int percentempty(Map * map)
{
  int i,j, empty, total;
  int xsize=map->xsize;
  int ysize=map->ysize;
  total=ysize*xsize;
  empty=0;
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (acceptabletodraw(j,i,map) == 1) {
      //      if (map->cango(j,i) == 0) {
	empty++;
      }
    }
  }
  return (100*empty)/total;
}

int enoughspace(int ly, int lx, int hy, int hx, Map * map)
{
  int i,j;
  for (i=lx;i<=hx;i++) {
    for (j=ly;j<=hy;j++) {
      if (acceptabletodraw(j,i,map) == 0) {
	return 0;
      }
    }
  }
  return 1;
}

int rectangularroom(int ly, int lx, int hy, int hx, Map * map)
{
  if (enoughspace(ly,lx,hy,hx,map) == 0) {return 0;}
  int k,l;
  for (k=lx;k<=hx;k++) {
    for (l=ly;l<=hy;l++) {
      map->settile(l,k,floortile);
    }
  }
  return 1;
}

int hollowroom(int ly, int lx, int hy, int hx, Map * map)
{
  if (hy-ly < 3 || hx-lx < 3) {return 0;}
  if (enoughspace(ly,lx,hy,hx,map) == 0) {return 0;}
  int k,l;
  for (k=lx;k<=hx;k++) {
    map->settile(ly,k,floortile);
    map->settile(hy,k,floortile);
  }
  for (l=ly;l<=hy;l++) {
    map->settile(l,lx,floortile);
    map->settile(l,hx,floortile);
  }
  return 1;
}

int caveroom(int ly, int lx, int hy, int hx, int py, int px, Map * map)
{
  if ((hx-lx) < 5 || (hy-ly) < 5) {return 0;}
  if (enoughspace(ly,lx,hy,hx,map) == 0) {return 0;}
  int i,j;
  for (i=lx;i<=hx;i++) {
    for (j=ly;j<=hy;j++) {
      if (mtrand()>0.5) {
	map->settile(j,i,floortile);
      }
    }
  }
  int reps=1;//(int)(mtrand()*3)+3;
  while (reps > 0) {
    for (i=lx;i<=hx;i++) {
      for (j=ly;j<=hy;j++) {
	if (checksurroundingtiles(j,i,map) > 5) {
	  map->seteffected(j,i,1);
	}
	else {
	  map->seteffected(j,i,0);
	}
      }
    }
    for (i=lx;i<=hx;i++) {
      for (j=ly;j<=hy;j++) {
	if (map->iseffected(j,i)==1) {
	  map->settile(j,i,walltile);
	  map->seteffected(j,i,0);
	}
	else {
	  map->settile(j,i,floortile);
	}
      }
    }
    reps--;
  }

  int cx,cy,dx,dy;
  cx = (hx+lx)/2;
  cy = (hy+ly)/2;
  if (cy>py) {dy=1;}
  else if (cy<py) {dy=-1;}
  else {dy=0;}
  if (cx>px) {dx=1;}
  else if (cx<px) {dx=-1;}
  else {dx=0;}
  while (py != cy || px != cx) {
    map->settile(py,px,floortile);
    if (mtrand()>0.5) {
      if (px != cx) {
	px+=dx;
      }
    }
    else {
      if (py != cy) {
	py+=dy;
      }
    }
  }

  return 1;
}

int curvyhall(int ly, int lx, int hy, int hx, int py, int px, int dy, int dx, Map * map)
{
  int xsize = (hx-lx)+1;
  int ysize = (hy-ly)+1;
  if (xsize<4 || ysize <4) {return 0;}
  int i,j;
  int ty,tx;
  int roomplan[ysize][xsize];
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      roomplan[j][i]=0;
    }
  }
  if (dy > 0) {ty=hy;}
  else if (dy < 0) {ty=ly;}
  else {ty = (int)(mtrand() *(hy-ly)) + ly;}
  if (dx > 0) {tx=hx;}
  else if (dx < 0) {tx=lx;}
  else {tx = (int)(mtrand() *(hx-lx)) + lx;}

  if (enoughspace(ty-4,tx-4,ty+4,tx+4,map) == 0) {return 0;}  

  px += dx; py += dy;

  while (px != tx || py != ty) {
    dx=0;dy=0;
    roomplan[py-ly][px-lx]=1;
    if (mtrand() > 0.5) {
      if (mtrand()*abs((float)(py-ty)) < 1) {
	if (py > ty) {py--;dy=-1;}
	else if (py < ty) {py++;dy=1;}
	else {dy=0;}
	continue;
      }
    }
    else {
      if (mtrand()*abs((float)(px-tx)) < 1) {
	if (px > tx) {px--;dx=-1;}
	else if (px < tx) {px++;dx=1;}
	else {dx=0;}
	continue;
      }
    }
  }

  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (roomplan[j][i] == 1) {
	if (acceptabletodraw(ly+j,lx+i,map) == 0) {
	  return 0;
	}
      }
    }
  }

  int tries=7;
  int lly,llx,hhx,hhy;
  while (tries > 0) {
    lly=ty+dy;llx=tx+dx;hhy=ty+dy;hhx=tx+dx;
    if (-dy>= 0) {lly -= (int)(mtrand()*7)+2;}
    if (-dx>= 0) {llx -= (int)(mtrand()*7)+2;}
    if (-dy<= 0) {hhy += (int)(mtrand()*7)+2;}
    if (-dx<= 0) {hhx += (int)(mtrand()*7)+2;}
    if (addroom(lly,llx,hhy,hhx,ty,tx,map,1) == 0) {tries--;}
    else {break;}
  }
  if (tries==0) {return 0;}

  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (roomplan[j][i] == 1) {
	map->settile(j+ly,i+lx,floortile);      
      }
    }
  }
  return 1;
}

int freeformroom(int ly, int lx, int hy, int hx, int py, int px, int dy, int dx, Map * map)
{
  if (hy-ly < 4 || hx-lx < 4) {return 0;}
  //  if (enoughspace(ly,lx,hy,hx,map) == 0) {return 0;}
  int xsize = (hx-lx+1)/2;
  int ysize = (hy-ly+1)/2;
  int i,j;
  int roomplan[ysize][xsize];
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      roomplan[j][i]=0;
    }
  }
  int total=xsize*ysize;
  int numfilled=1;
  i=(int)(mtrand()*xsize);
  j=(int)(mtrand()*ysize);
  roomplan[j][i]=1;
  while (numfilled<total/2) {
    i+=(int)(mtrand()*3)-1;
    j+=(int)(mtrand()*3)-1;
    if (i<0) {i=0;}
    if (i>=xsize) {i=xsize-1;}
    if (j<0) {j=0;}
    if (j>=ysize) {j=ysize-1;}
    if (roomplan[j][i]==0) {
      roomplan[j][i]=1;
      numfilled++;
    }
  }

  for (i=0;i<=(hx-lx);i++) {
    for (j=0;j<=(hy-ly);j++) {
      if (roomplan[j/2][i/2] == 1) {
	if (acceptabletodraw(ly+j,lx+i,map) == 0) {
	  return 0;
	}
      }
    }
  }


  for (i=0;i<=(hx-lx);i++) {
    for (j=0;j<=(hy-ly);j++) {
      if (roomplan[j/2][i/2] == 1) {
	map->settile(j+ly,i+lx,floortile);	
      }
    }
  }

  for (i=0;i<=(hx-lx);i++) {
    for (j=0;j<=(hy-ly);j++) {
      if (checksurroundingtiles(j+ly,i+lx,map)==4) {
	map->seteffected(j+ly,i+lx,1);
      }
    }
  }
  
  
  for (i=0;i<=(hx-lx);i++) {
    for (j=0;j<=(hy-ly);j++) {
      if (map->inside(j+ly,i+lx) == 1) {
	if (map->iseffected(j+ly,i+lx) == 1) {
	  map->settile(j+ly,i+lx,floortile);
	  map->seteffected(j+ly,i+lx,0);
	}
      }
    }
  }

  int cx,cy;
  cx = (hx+lx)/2;
  cy = (hy+ly)/2;
  while (map->cango(cy,cx)==0) {
    cy = ly + (int)(mtrand() * (hy-ly));
    cx = lx + (int)(mtrand() * (hx-lx));
  }
  if (cy>py) {dy=1;}
  else if (cy<py) {dy=-1;}
  else {dy=0;}
  if (cx>px) {dx=1;}
  else if (cx<px) {dx=-1;}
  else {dx=0;}
  while ((py != cy || px != cx) && map->cango(py,px)==0) {
    map->settile(py,px,floortile);
    if (px != cx) {
      px+=dx;
      continue;
    }
    if (py != cy) {
      py+=dy;
      continue;
    }
  }


  return 1;
}

int ovalroom(int ly, int lx, int hy, int hx, int py, int px, Map * map)
{
  int xradius,yradius,mr,rx,ry,r;
  xradius = (hx-lx)/2;
  yradius = (hy-ly)/2;
  if (xradius < 3 || yradius < 3) {return 0;}
  mr=xradius;
  if (mr < yradius) {mr=yradius;}
  int t,x,y;

  // Check if it will fit.
  for (r=0;r<=mr;r++) {
    for (t=0;t<=6*mr;t++) {
      rx=r; ry=r;
      if (rx > xradius) {rx=xradius;}
      if (ry > yradius) {ry=yradius;}
      x=lx+(xradius+(int)(((float)rx)*cos(2*PI*((float)t)/((float)mr*6))+0.5));
      y=ly+(yradius+(int)(((float)ry)*sin(2*PI*((float)t)/((float)mr*6))+0.5));
      if (acceptabletodraw(y,x,map) == 0) {
	return 0;
      }
    }
  }

  // Draw it.
  for (r=0;r<=mr;r++) {
    for (t=0;t<=6*mr;t++) {
      rx=r; ry=r;
      if (rx > xradius) {rx=xradius;}
      if (ry > yradius) {ry=yradius;}
      x=lx+(xradius+(int)(((float)rx)*cos(2*PI*((float)t)/((float)mr*6))+0.5));
      y=ly+(yradius+(int)(((float)ry)*sin(2*PI*((float)t)/((float)mr*6))+0.5));
      map->settile(y,x,floortile);
    }
  }
  int cx,cy,dx,dy;
  cx = (hx+lx)/2;
  cy = (hy+ly)/2;
  if (cy>py) {dy=1;}
  else if (cy<py) {dy=-1;}
  else {dy=0;}
  if (cx>px) {dx=1;}
  else if (cx<px) {dx=-1;}
  else {dx=0;}
  while (py != cy || px != cx) {
    map->settile(py,px,floortile);
    if (mtrand()>0.5) {
      if (px != cx) {
	px+=dx;
      }
    }
    else {
      if (py != cy) {
	py+=dy;
      }
    }
  }
  return 1;
}

int longhallway(int dy, int dx, int py, int px, Map * map)
{
  int ly,lx,hy,hx,length,roomx,roomy;
  roomx=6;roomy=6;
  ly = py+dy;
  hy = py+dy;
  lx = px+dx;
  hx = px+dx;
  length=(int)(7*mtrand())+3;
  if (dy > 0) {hy+=length;}
  if (dy < 0) {ly-=length;}
  if (dx > 0) {hx+=length;}
  if (dx < 0) {lx-=length;}
  if (enoughspace(ly,lx,hy,hx,map) == 0) {return 0;}

  // Try to add a room at the end of the hall
  int numberofattempts=7;
  int x=px+dx*length;
  int y=py+dy*length;
  //  int ly,lx,hy,hx;
  while (numberofattempts>0) {
    ly=y+dy;lx=x+dx;hy=y+dy;hx=x+dx;
    if (-dy>= 0) {ly -= (int)(mtrand()*7)+2;}
    if (-dx>= 0) {lx -= (int)(mtrand()*7)+2;}
    if (-dy<= 0) {hy += (int)(mtrand()*7)+2;}
    if (-dx<= 0) {hx += (int)(mtrand()*7)+2;}
    if (addroom(ly,lx,hy,hx,y,x,map,1) == 0) {numberofattempts--;}
    else {break;}
  }
  if (numberofattempts==0) {return 0;}

  while (length > 0) {
    map->settile(py,px,floortile);
    py+=dy;
    px+=dx;
    length--;
  }
  
  return 1;
}

int connectinghallway(int dy, int dx, int py, int px, Map * map)
{
  if (map->inside(py,px)==0) {return 0;}
  if (map->cango(py,px)!=0) {return 0;}
  int minlength=(int)(7*mtrand())+3;
  int maxlength=minlength+3;
  int i,x,y,j,turnat,dir;
  turnat = (int)(2*minlength*mtrand())+1;
  dir = (int)(2*mtrand())*2-1;
  i=1;
  y=py+dy;
  x=px+dx;
  int tdy,tdx;
  tdy=dy; tdx=dx;

  while (i<minlength && acceptabletodraw(y,x,map)==1) {
    if (i==turnat) {tdy=dir*dx; tdx=dir*dy;}
    y+=tdy;
    x+=tdx;
    i++;
  }
  if (i<minlength) {return 0;}

  i=1;
  y=py+dy;
  x=px+dx;
  tdy=dy; tdx=dx;

  while (i<maxlength) {
    if (i==turnat) {tdy=dir*dx; tdx=dir*dy;}
    if (acceptabletodraw(y,x,map)==0) {break;}
    y+=tdy;
    x+=tdx;
    i++;
  }

  // if the hall ends next to another room, they should hook up!
  if (map->cango(y+2*tdy,x+2*tdx) == 1) {
    map->settile(y+tdy,x+tdx,floortile);
  }
  else {return 0;}

  j=i;
  i=0;
  y=py;
  x=px;
  tdy=dy; tdx=dx;

  while (i<=j) {
    map->settile(y,x,floortile);
    if (i==turnat) {tdy=dir*dx; tdx=dir*dy;}
    y+=tdy;
    x+=tdx;
    i++;
  }
  
  return 1;
}

// Draw path from one point to another
void path(int sy, int sx, int ty, int tx, Map * map, int thickness, int type, tile tiletoplace, bool overall) {
  int dx,dy,i,j;
  int abletogo=tiletoplace.passable;
  if (overall) {
    abletogo=2; // Draw on everything! EVERYTHING.
  }
  bool dox;
  if (ty > sy) {dy=1;}
  else {dy=-1;}
  if (tx > sx) {dx=1;}
  else {dx=-1;}
  if (map->cango(sy,sx)!=abletogo) {
    if (map->inside(sy,sx) == 1) {
      if (map->getfloorsymb(sy,sx) != '>' && map->getfloorsymb(sy,sx) != '<') {
	map->settile(sy,sx,tiletoplace);
      }
    }
  }
  while (sy != ty || sx != tx) {
    switch(type) {
    case 3: {dox = ((abs(tx-sx) != 0 && mtrand()>0.5) || abs(ty-sy) == 0); break;} // shitty and dumb
    case 2: {dox = (abs(tx-sx) != 0); break; } // manhattan style
    case 1: {dox = ((int)((abs(ty-sy) + abs(tx-sx))*mtrand()+1) > abs(ty-sy)); break; } // curvy
    case 0:
    default: {dox = (abs(tx-sx) > abs(ty-sy)); break;} // diagonals included
    }
    if (dox) {
      sx += dx;
    }
    else {
      sy += dy;
    }
    for (i=0;i<thickness;i++) {
      for (j=0;j<thickness;j++) {
	if (map->cango(sy-j*dy,sx-i*dx)!=abletogo) {
	  if (map->inside(sy-j*dy,sx-i*dx) == 1) {
	    if (map->getfloorsymb(sy-j*dy,sx-i*dx) != '>' && map->getfloorsymb(sy-j*dy,sx-i*dx) != '<') {
	      map->settile(sy-j*dy,sx-i*dx,tiletoplace);
	    }
	  }
	}
      }
    }
  }
}

void path(int sy, int sx, int ty, int tx, Map * map, int thickness, int type, tile tiletoplace) {
  path(sy, sx, ty, tx, map, thickness, type, floortile,0);
}

void path(int sy, int sx, int ty, int tx, Map * map) {
  path(sy, sx, ty, tx, map, 1, 0, floortile);
}


// Mark all contiguous passable squares effected
int filleffected(int y, int x, Map * map) {
  if (map->cango(y,x)==0) {return 0;}
  if (map->iseffected(y,x) == 1) {return 0;}
  map->seteffected(y,x,1);
  int i,j;
  for (i=-1;i<2;i++) {
    for (j=-1;j<2;j++) {
      if (j != 0 && i != 0) {continue;} // Only pay attention to cardinal directions
      if (map->cango(y+j,x+i) == 1) {
	if (map->iseffected(y+j,x+i) == 0) {
	  filleffected(y+j,x+i,map);
	}
      }
    }
  }
  return 1;
}

// Place items and populate rooms
int placement(int ly, int lx, int hy, int hx, int y, int x, char itemprob, char monprob, char type, Map * map)
{
  if (map->cango(y,x) == 0 || map->canshoot(y,x) == 0) {return 0;}
  if ((y < ly || y > hy) || (x < lx || x > hx)) {return 0;}
  if (map->iseffected(y,x) == 1) {return 0;}
  if (y != (ly+hy)/2 || x != (lx+hx)/2) {
    if ((int)(100*mtrand()) < itemprob) {
      placeitem(y, x, map);
    }
    if ((int)(100*mtrand()) < monprob) {
      map->placemonster(y, x);
    }
  }
  map->seteffected(y,x,1);
  int i, j;
  for (i=-1;i<2;i++) {
    for (j=-1;j<2;j++) {
      if (map->cango(y+j,x+i) == 1) {
	if (map->iseffected(y+j,x+i) == 0) {
	  placement(ly,lx,hy,hx,y+j,x+i,itemprob,monprob,1,map);
	}
      }
    }
  }
  if (type == 0) {
    for (i=lx;i<=hx;i++) {
      for (j=ly;j<=hy;j++) {
	if (map->inside(j,i)==1) {
	  map->seteffected(j,i,0);
	}
      }
    }
  }
  return 1;
}

int placement(int ly, int lx, int hy, int hx, int y, int x, char itemprob, char monprob, Map * map)
{
  placement(ly,lx,hy,hx,y,x,itemprob,monprob,0,map);
}

// Make a new room
// lower y and x, higher y and x, parent y and x, map
int addroom(int ly, int lx, int hy, int hx, int py, int px, Map * map)
{
  return addroom(ly, lx, hy, hx, py, px, map, 0);
}

int addroom(int ly, int lx, int hy, int hx, int py, int px, Map * map, int mustberoom)
{
  int roomtype;
  int success;

  int llastroomy, llastroomx;
  llastroomy=lastroomy;
  llastroomx=lastroomx;

  int dy,dx;
  if ((ly-py)==1) {
    dy=1;dx=0;
  }
  else if ((py-hy)==1) {
    dy=-1;dx=0;
  }
  else if ((lx-px)==1) {
    dy=0;dx=1;
  }
  else {
    dy=0;dx=-1;
  }


  roomtype = 18*mtrand()+1;
  if (mustberoom != 0) {
    while (roomtype == 12 || roomtype == 10 || roomtype == 13 || roomtype == 14) {
      roomtype = 12*mtrand()+1;
    }
  }
  switch(roomtype) {
  case 15:
  case 16: {
    success=freeformroom(ly,lx,hy,hx,py,px,dy,dx,map);
    break;
  }
  case 13:
  case 14: {
    success=curvyhall(ly,lx,hy,hx,py,px,dy,dx,map);
    break;
  }
  case 12:
  case 10:{
    success=longhallway(dy,dx,py,px,map);
    break;
  }
  case 7:
  case 8:
  case 9: {
    success=caveroom(ly,lx,hy,hx,py,px,map);
    break;
  }
  case 6:
  case 4:
  case 5: {
    success=ovalroom(ly,lx,hy,hx,py,px,map);
    break;
  }
  case 2:
  case 1: {
    success=rectangularroom(ly,lx,hy,hx,map);
    break;
  }
  case 11:
  case 3: {
    success=hollowroom(ly,lx,hy,hx,map);
    break;
  }
  case 18:
  case 17: {
    success=connectinghallway(dy,dx,py,px,map);
    break;
  }
  }
  if (success==1) {
    if ((roomtype != 12 && roomtype != 10 && roomtype != 13 && roomtype != 14 && roomtype != 18 && roomtype != 17)) {
      int y,x;
      y = (int)((hy-ly)*mtrand())+ly;
      x = (int)((hx-lx)*mtrand())+lx;
      while (placement(ly, lx, hy, hx, y, x, 5, 2, map) == 0) {
      	y = (int)((hy-ly)*mtrand())+ly;
      	x = (int)((hx-lx)*mtrand())+lx;	
      }
      if (llastroomy == lastroomy && llastroomx == lastroomx) {
	lastroomy = (hy+ly)/2;
	lastroomx = (hx+lx)/2;
      }
    }
    map->settile(py,px,doortile);
  }
  return success;
}

void buildacave(Map * map) {
  int planx,plany;
  int xsize,ysize;
  int i,j;
  xsize=map->xsize; ysize=map->ysize;

  planx = xsize / PLANRES;
  plany = ysize / PLANRES;
  int largeplan[plany][planx];
  for (i=0;i<planx;i++) {
    for (j=0;j<plany;j++) {
      largeplan[j][i]=0;
    }
  }
  i=(int)(mtrand()*planx);
  j=(int)(mtrand()*plany);
  int numsquares = 4+(int)(mtrand()*4);
  int dd=0;
  while (numsquares>=0) {
    if (largeplan[j][i] == 0) {largeplan[j][i]=1;numsquares--;}
    dd = 2*(int)(mtrand()*2)-1;
    if (mtrand()>0.5) {
      i+=dd;
    }
    else {
      j+=dd;
    }
    if (i<0) {i=0;}
    if (i>=planx) {i=planx-1;}
    if (j<0) {j=0;}
    if (j>=plany) {j=plany-1;}
  }
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (largeplan[j/PLANRES][i/PLANRES]==1) {
	if (mtrand()>0.5) {
	  map->settile(j,i,floortile);
	}
      }
    }
  }
  
  dd=3;
  while (dd>=0) {
    for (i=0;i<xsize;i++) {
      for (j=0;j<ysize;j++) {
	if (checksurroundingtiles(j,i,map) >= 5) {
	  map->seteffected(j,i,1);
	}
      }
    }
    
    for (i=0;i<xsize;i++) {
      for (j=0;j<ysize;j++) {
	if (map->iseffected(j,i)==1) {
	  map->settile(j,i,walltile);
	  map->seteffected(j,i,0);
	}
	else {
	  map->settile(j,i,floortile);
	}
      }
    }
    
    
    dd--;
  }
  
}

void hallnetwork(Map * map, int type) {
  int sx,sy,tx,ty;
  int numnodes=5*mtrand()+3;
  int thickness=4*mtrand()+1;
  int xsize=map->xsize;
  int ysize=map->ysize;
  sy=(int)((ysize-10)*mtrand())+5;
  ty=(int)((ysize-10)*mtrand())+5;
  sx=(int)((xsize-10)*mtrand())+5;
  tx=(int)((xsize-10)*mtrand())+5;
  while (numnodes > 0) {
    path(sy, sx, ty, tx, map, thickness, type, floortile);

    sx=tx; sy=ty;

    ty=(int)((ysize-10)*mtrand())+5;
    tx=(int)((xsize-10)*mtrand())+5;

    numnodes--;
  }
}

void hallnetwork(Map * map) {
  hallnetwork(map, 0);
}

void chasm(Map * map) {
  int xsize = map->xsize;
  int ysize = map->ysize;
  int crevys=0;
  int crevxs=0;
  int crevye=ysize;
  int crevxe=xsize;
  
  if (mtrand()>0.5) {
    crevys=(int)(ysize*mtrand());
    crevye=(int)(ysize*mtrand());
  }
  else {
    crevxs=(int)(xsize*mtrand());
    crevxe=(int)(xsize*mtrand());
  }
  
  path(crevys, crevxs, crevye, crevxe, map, 8, 1, floortile,1);
  path(crevys-1, crevxs-1, crevye-1, crevxe-1, map, 6, 1, holetile,1);
}

int Map::newgenmap(llist ** mmlist, int *sy, int *sx, Map *prevmap)
{

  int i,j,k,l,x,y;
  llist * processes=NULL;
  llist * templist=NULL;
  coords * position;

  if (prevmap != NULL) {x=prevmap->xsize; y=prevmap->ysize;}
  else {x=2;y=2;}
  int plan[y][x];
  for (i=0;i<x;i++) {
    for (j=0;j<y;j++) {
      plan[j][i]=0;
      if (prevmap != NULL) {
	if (prevmap->getfloorsymb(j,i) == '>') {
	  plan[j][i]=1;
	}
	if (prevmap->getfloorsymb(j,i) == ' ' || prevmap->getfloorsymb(j,i) == '=') {
	  plan[j][i]=2;
	}
      }
    }
  }

  if (map != NULL) {this->deallocatemap();}

  maxpop = dlvl*10; // Set maximum monster population
  this->allocatemap(0,0); // Allocate the space for the map
  this->mlist = mmlist; // Monster list

  walltile.passable=0;
  walltile.symbol = '#';
  walltile.seensymbol = ' ';
  walltile.seen=0;
  walltile.visible=0;
  walltile.seethrough=0;
  walltile.shootthrough=0;
  walltile.bloody=0;
  walltile.opacity=2;
  walltile.creature=NULL;
  walltile.ilist=NULL;
  walltile.color=BROWN;
  walltile.effected=0;

  floortile=walltile;
  floortile.passable=1;
  floortile.seethrough=1;
  floortile.shootthrough=1;
  floortile.color=WHITE;
  floortile.symbol='.';

  holetile=floortile;
  holetile.symbol=' ';
  holetile.passable=0;

  doortile=floortile;
  doortile.symbol='+';
  doortile.shootthrough=0;
  doortile.seethrough=0;

  // Fill with impassable wall.
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      map[j][i]=walltile;
    }
  }

  int foundstairs=0;
  int success=0;
  int ly,lx,hy,hx;
  int whichfeature=(int)(6*mtrand());

  // Chance of adding major features (definite for the time being. make less likely later)
  if (1>0 && whichfeature < 5) {
    
    switch (whichfeature) {
    case 0: // Diagonal hall network
    case 1: // Curvy hall network
    case 2: // Cardinal only hall network
    case 3: { // Shitty and dumb hall network
      hallnetwork(this,whichfeature);
      break;
    }
    case 4: {
      buildacave(this); // Cavern!
      break;
    }
    }

  }
  // End of major feature adding


  // Locate staircases. If none exist, just pick a random start point.
  if (prevmap != NULL) {
    // Build around stairs.
    for (i=0;i<xsize;i++) {
      for (j=0;j<ysize;j++) {
	if (plan[j][i] == 2) {
	  if (map[j][i].symbol != ' ') {
	    this->settile(j,i,floortile);
	  }
	}
	if (plan[j][i] == 1) {
	  if (this->cango(j,i)==0) {
	    success=10;
	    foundstairs++;
	    while (success > 0) {
	      ly=j-(int)(6*mtrand())-1;
	      lx=i-(int)(6*mtrand())-1;
	      hx=i+(int)(6*mtrand())+1;
	      hy=j+(int)(6*mtrand())+1;
	      success--;
	      if (enoughspace(ly,lx,hy,hx,this) == 1) {success=-1;}
	    }
	    if (success==-1)  {
	      for (k=lx;k<=hx;k++) {
		for (l=ly;l<=hy;l++) {
		  this->settile(l,k,floortile);
		}
	      }
	    }
	    this->settile(j,i,floortile);
	    map[j][i].symbol = '<';
	  }
	}
      }
    }
  }
  if (foundstairs == 0) {
    if (*sx < 0 || *sy < 0) {
      this->rpos(&y,&x);
      *sx=x;
      *sy=y;
    }
    else {x=*sx;y=*sy;}
    success=10;
    // Only add a starting room if it is necessary.
    if (this->cango(y,x)==0) {
      while (success > 0) {
	ly=y-(int)(5*mtrand())-1;
	lx=x-(int)(5*mtrand())-1;
	hx=x+(int)(5*mtrand())+1;
	hy=y+(int)(5*mtrand())+1;
	success--;
	if (enoughspace(ly,lx,hy,hx,this) == 1) {success=-1;}
      }
      if (success==-1) {
	for (k=lx;k<=hx;k++) {
	  for (l=ly;l<=hy;l++) {
	    this->settile(l,k,floortile);
	  }
	}
      }
    }
    this->settile(y,x,floortile);
    placeweapon(y,x);
    map[y][x].symbol = '<';
  }

  int tilgivingup=100;
  // While the map is mostly empty... keep filling it!
  while (percentempty(this) > 40 && tilgivingup > 0) {
    // Choose a random position
    this->rpos(&y,&x);
    // Find number of neighbouring walls
    int neighbours = checksurroundingtiles(y,x,this);
    int numberofattempts=10;
    // If it's in the middle of a room, random walk until something decent is found.
    while ((neighbours != 5 && neighbours != 7) || this->cango(y,x) == 1) {
      y+=(int)(mtrand()*3)-1;
      x+=(int)(mtrand()*3)-1;
      if (y<0) {y=1;}
      if (x<0) {x=1;}
      if (y>=ysize) {y=ysize-1;}
      if (x>=xsize) {x=xsize-1;}
      neighbours = checksurroundingtiles(y,x,this);
      if (numberofattempts==0) {neighbours=8;break;}
      if (neighbours == 8) {break;}
      numberofattempts--;
    }
    // If it's in the middle of nowhere, don't even bother.
    if (neighbours == 8) {continue;}

    success=0;
    for (i=-1;i<2;i++) {
      for (j=-1;j<2;j++) {
	if ((i==0 || j==0) && (i != 0 || j != 0)) {
	  if (this->inside(y+j,x+i) == 1) {
	    if (this->cango(y+j,x+i) == 1) {
	      success=1;
	      break;
	    }
	  }
	}
      }
      if (success==1) {break;}
    }
    if (success==0) {continue;}
    numberofattempts=5;
    // Set outer bounds to the found point... expand outwards!
    while (numberofattempts>0) {
      ly=y;lx=x;hy=y;hx=x;
      ly-=j;hy-=j;lx-=i;hx-=i;
      if (j>= 0) {ly -= (int)(mtrand()*8)+1;}
      if (i>= 0) {lx -= (int)(mtrand()*8)+1;}
      if (j<= 0) {hy += (int)(mtrand()*8)+1;}
      if (i<= 0) {hx += (int)(mtrand()*8)+1;}
      if (addroom(ly,lx,hy,hx,y,x,this) == 0) {numberofattempts--;}
      else {
	tilgivingup=100;break;
      }
    }
    tilgivingup--;

  }

  //  this->rpos(&lastroomy,&lastroomx);
  if (this->cango(lastroomy,lastroomx)==0 || this->canshoot(lastroomy,lastroomx)==0) {
    this->settile(lastroomy, lastroomx, floortile);
  }
  map[lastroomy][lastroomx].symbol = '>';

  if (whichfeature >= 5) {
    chasm(this); // Chasm!
  }

  // Check for connectedness and rectify if not connected.
  // This should be made into a function.
  int attempttofix=1;
  // good x, good y, bad x, bad y, and possibilities for each
  int gx,gy,bx,by, pgx,pgy,pbx,pby;
  while (attempttofix==1) {
    gx=-1;gy=-1;bx=-1;by=-1;
    pgx=-1;pgy=-1;pbx=-1;pby=-1;
    attempttofix=0;
    while (filleffected(y,x,this)==0) {
      this->rpos(&y,&x);
    }
    for (i=1;i<xsize-1;i++) {
      for (j=1;j<ysize-1;j++) {
	if (this->cango(j,i) == 1) {
	  if (this->iseffected(j,i)==1) {
	    pgx=i;pgy=j;
	    this->seteffected(j,i,0);
	  }
	  else {
	    attempttofix=1;
	    pbx=i;pby=j;
	  }
	}

	if (pbx > 0 && pgx > 0) {
	  if ( gx > 0 && bx > 0) {
	    if (abs(pbx-gx)+abs(pby-gy) < abs(bx-gx)+abs(by-gy)) {
	      bx=pbx; by=pby;
	    }
	    else if (abs(bx-pgx)+abs(by-pgy) < abs(bx-gx)+abs(by-gy)) {
	      gx=pgx; gy=pgy;
	    }
	    else if (abs(pbx-pgx)+abs(pby-pgy) < abs(bx-gx)+abs(by-gy)) {
	      gx=pgx; gy=pgy; bx=pbx; by=pby;
	    }
	  }
	  else {
	    gx=pgx; gy=pgy; bx=pbx; by=pby;
	  }
	}
      }
    }
    if (attempttofix==1 && ((bx >= 0 && gx >= 0))) {
      // Connected the last found good tile with the last found bad tile.
      path(by, bx, gy, gx, this,1,2,floortile);
    }
  }
  
  // Surround the map with rock, just in case.
  for (j=0;j<=1;j++) {
    for (i=0;i<xsize;i++) {
      if (map[j*(ysize-1)][i].symbol == ' ') {
	map[j*(ysize-1)][i].seethrough = 0;
	map[j*(ysize-1)][i].shootthrough = 0;
      }
      else {
	map[j*(ysize-1)][i] = walltile;
      }
    }
  }

  for (j=0;j<=1;j++) {
    for (i=0;i<ysize;i++) {
      if (map[i][j*(xsize-1)].symbol == ' ') {
	map[i][j*(xsize-1)].seethrough = 0;
	map[i][j*(xsize-1)].shootthrough = 0;
      }
      else {
	map[i][j*(xsize-1)] = walltile;
      }
    }
  }

  // various postprocessing.
  for (i=1;i<xsize-1;i++) {
    for (j=1;j<ysize-1;j++) {
      //Get rid of stupid doors.
      if (map[j][i].symbol == '+') {
	if (!((this->cango(j-1,i) == 0 && this->cango(j+1,i) == 0) || (this->cango(j,i-1) == 0 && this->cango(j,i+1) == 0))) {
	  map[j][i]=floortile;
	}
      }
      // Make floortiles on chasms into bridges
      if (map[j][i].symbol == '.') {
	if (((this->getfloorsymb(j-1,i) == ' ' && this->getfloorsymb(j+1,i) == ' ') || (this->getfloorsymb(j,i-1) == ' ' && this->getfloorsymb(j,i+1) == ' '))) {
	  map[j][i].symbol='=';
	  map[j][i].color=BROWN;
	}
      }
    }
  }
  
  // Make walls visible, for debugging purposes
  /*for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (this->cango(j,i)==0) {
	if (checksurroundingtiles(j,i,this) < 8) {
	  map[j][i].seen=1;
	  map[j][i].seensymbol='#';
	}
      }
    }
  }*/
  
}
