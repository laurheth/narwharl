/*
NarwhaRL - Nice And Relaxing or Worst Hell Always? the RogueLike
Copyright (C) 2010-2018 Lauren Hetherington

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

 */


// Map generation shit.

#include <iostream>
#include "nheader.h"
//#include <ncurses/cursesapp.h>
#include <cstring>
#include <fstream>

using namespace std;

extern Stuffgetter * objget;

// Random start position.
void Map::rpos(int *y, int *x)
{
  *x = (int)((xsize-3)*mtrand()+2);
  *y = (int)((ysize-3)*mtrand()+2);
}

// Make a room at position (y,x), with 1->4 exits at roomdepth. If roomdepth=0 then stop.
int Map::mkroom(int y, int x, int exits, int roomdepth)
{
  if (exits == 0 && roomdepth > 0) {exits=(int)(4*mtrand());}
  int i,j;
  int sizey,sizex;
  sizex = (int)(3*mtrand()+3); // Random size 3->5
  sizey = (int)(3*mtrand()+3);
  for (i=(x-sizex/2);i<(x+sizex/2)+1;i++) {
    for (j=(y-sizey/2);j<(y+sizey/2)+1;j++) {
      if ((i>0 && i<(xsize-1)) && (j>0 && j<(ysize-1))) {
	if (map[j][i].passable != 1) {
	  map[j][i].passable = 1; // Make it passable
	  map[j][i].symbol = '.'; // Give it floor symbol.
	  map[j][i].opacity = 2; // Give it regular opacity.
	  map[j][i].seethrough = 1; // Make it seethrough
	  map[j][i].color=WHITE; // Make it white.
	  // Populate room.
	  if (mtrand()>0.98) {
	    //	    placemonster(j, i);
	  }
	  if (mtrand()>0.95) {
	    placeweapon(j, i);
	  }
	  if (mtrand()>0.95) {
	    placescroll(j, i);
	  }
	  if (mtrand()>0.95) {
	    placearmor(j, i);
	  }
	  if (mtrand()>0.95) {
	    placepotion(j, i);
	  }
	  if (mtrand()>0.95) {
	    placewand(j, i);
	  }
	  if (mtrand()>0.95) {
	    placelauncher(j, i);
	  }
	  if (mtrand()>0.95) {
	    placeammo(j, i);
	  }
	  // Split off hallways from random positions.
	  if (mtrand()>0.8 && exits > 0) {
	    hallway(j,i,0,roomdepth);
	  }
	}
      }
    }
  }
  // If exits were not place, do them starting from the centre tile.
  if (exits>0) {
    for (i=0;i<exits;i++) {
      hallway(y,x,i+1,roomdepth);
    }
  }
  return 1;
}

// Draw hallway
int Map::hallway(int y, int x, int dir, int roomdepth)
{
  int i;
  int length;
  int dy,dx;
  length = 5*(int)(5*mtrand()+5); // Random length.
  if (dir < 1 || dir > 4) {dir = (int)(4*mtrand()+1);}
  // Set dx and dy according to direction.
  switch (dir) {
  case 1: dy=1;dx=0;break;
  case 2: dy=-1;dx=0;break;
  case 3: dy=0;dx=-1;length=length;break;
  case 4: dy=0;dx=1;length=length;break;
  }
  for (i=0;i<length;i++) {
    y = y+dy; // Choose next tile.
    x = x+dx;
    if ((x>0 && x<(xsize-1)) && (y>0 && y<(ysize-1))) {
      if (map[y][x].passable != 1) {
	map[y][x].passable = 1; // Set floor properties.
	map[y][x].symbol = '.';
	map[y][x].opacity = 2;
	map[y][x].seethrough = 1;
	map[y][x].color=WHITE;
	if (mtrand()>0.98) { // MAYBE place monster. Probably not though.
	  //	  placemonster(y, x);
	}
      }
    }
    else {dx=-dx;dy=-dy;x=x+dx;y=y+dy; // If you reach the edge of the screen, change direction.
      int dt;
      if (mtrand() > 0.5) {dt=dx; dx=dy; dy=dt;}
      else {dt=dx; dx=-dy; dy=-dt;}
    }
  }
  dir = (int)(10*mtrand())+1;
  if (dir < 3) {
    hallway(y,x,0, roomdepth); // Maybe start another hallway.
  }
  else {
    mkroom(y,x,0,roomdepth-1); // Otherwise make a new room.
  }
  return 1;
}

// Start the map generation
int Map::genmap(llist ** mmlist, int *sy, int *sx)
{

  // Fill the map with solid rock.
  int i,j,x,y;
  int roomdepth;
  maxpop=dlvl*10;
  this->allocatemap(0,0); // Allocate map space.
  this->mlist=mmlist; // Set monster list.
  roomdepth = 4; // Starting max roomdepth. This is the number of times it branches.
  // Fill with immpassable walls.
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      map[j][i].passable = 0;
      map[j][i].symbol = '#';
      map[j][i].seen = 0;
      map[j][i].visible = 0;
      map[j][i].seethrough = 0;
      map[j][i].bloody = 0;
      map[j][i].opacity = 2;
      map[j][i].creature = NULL;
      map[j][i].ilist = NULL;
      map[j][i].color=BROWN;
    }
  }

  if (*sy < 0 || *sx < 0) {
    // Choose random start position.
    rpos(&y,&x);
  }
  else {
    x = *sx;
    y = *sy;
  }

  // Start room. It recursively progresses from there.
  mkroom(y,x,(int)(4*mtrand()+1),roomdepth);
  map[y][x].symbol = '<';
  map[y][x+1].symbol = '>';
  *sy = y;
  *sx = x;

  return 1;

}

void Map::placemonster(int y, int x)
{
  int tryagain=10; // Max number of tries.
  int maxcreaturelevel=dlvl+plvl-1;
  while (tryagain>0) {
    // Add a random creature using the Stuffgetter, if the square is empty.
    if (this->cango(y,x) == 1 && this->checkmonster(y,x) == NULL) {
      tryagain=0;
      Creature * creature;
      creature = objget->addcreature(maxcreaturelevel); // Creatre creature.
      creature->y=y; // Set position
      creature->x=x;
      this->go(creature, y, x);
      
      // Add to the action queue.
      list_add(this->mlist,creature);
      pop++;
    }
    else {tryagain--; y += (int)(3*mtrand())-1; x += (int)(3*mtrand())-1;} // Random walk until a suitable place is found, to a max of 10 tries.
  }
}

void Map::placeweapon(int y, int x)
{
  // Add a random weapon using the Stuffgetter
  Item * item;
  item = objget->addweapon(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placescroll(int y, int x)
{
  // Add a random scroll using the Stuffgetter
  Item * item;
  item = objget->addscroll(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placewand(int y, int x)
{
  // Add a random wand using the Stuffgetter
  Item * item;
  item = objget->addwand(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placearmor(int y, int x)
{
  // Add a random armor using the Stuffgetter
  Item * item;
  item = objget->addarmor(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placepotion(int y, int x)
{
  // Add a random potion using the Stuffgetter
  Item * item;
  item = objget->addpotion(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placelauncher(int y, int x)
{
  // Add a random launcher using the Stuffgetter
  Item * item;
  item = objget->addlauncher(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
}

void Map::placeammo(int y, int x)
{
  // Add a random launcher using the Stuffgetter
  Item * item;
  item = objget->addmissile(dlvl);

  // Set its location and put it on the map.
  item->y=y;
  item->x=x;
  this->put(item, y, x);
  int i;
  i = 6 * mtrand();
  if (i > 0) {
    Item * tostack;
    while (i > 0) {
      tostack = objget->addmissile(item->consumes);
      this->put(tostack, y, x);
      i--;
    }
  }
}
