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


#include <iostream>
#include "nheader.h"
#include <sstream>
#include <cmath>

using namespace std;

extern string knowledge;

// Monster constructor
Monster::Monster(Map *map, int sy, int sx, char ssymb, string sname) : Creature(map, sy, sx, ssymb, sname)
{
  targetx=-1;
  targety=-1;
  numberofspells=-1;
  active=0;
  plannedpath=NULL;
}

// Monster levelling up
void advancemon(Creature * creature, int levels)
{

  int i,j,k;
  for (i=0;i<levels;i++) {
    
    for (j=0;j<2;j++) {
      k = (int)(4*mtrand());
      switch (k) {
      case 0:
	creature->strength++; break;
      case 1:
	creature->dexterity++; break;
      case 2:
	creature->agility++; break;
      case 3:
	creature->toughness++; break;
      }
    }
    creature->level++;
    creature->maxmp++;
    
    
    for (j=0;j<NUMSKILLS;j++) {
      
      if (creature->skills[j] > 0) {
	creature->skills[j] += 1;
      }
      
    }
    
  }
  
  for (i=0;i<levels;i++) {
    j = (int)(NUMSKILLS * mtrand());
    creature->skills[j] += 1;
  }

  creature->maxhp = (creature->level)*(creature->toughness);
  creature->hp = creature->maxhp;
  
}


// Redo pathfinding.
void Monster::redopath(Map *map, Screen * vscreen)
{
  if (plannedpath != NULL) {
    while (plannedpath != NULL) {
      free((coords*)list_item(&plannedpath));
      list_remove(&plannedpath);
    }
  }
  findapath(y,x,targety,targetx,map,&plannedpath, vscreen);
}

// Monster ai details.
// Stuff like number of spells, potions, inventory... For use in ai()
void Monster::details()
{
  numberofspells=0;
  llist * slist;
  slist = *(this->spells());
  while (slist != NULL) {
    numberofspells++;
    slist=*(&slist->next);
  }
}

// Monster ai definition

int Monster::ai(Map *map, Screen *vscreen)
{

  string message;

  if (experience > XPCURVE * level) {
    experience -= XPCURVE * level;
    advancemon((Creature*)this, 1);
    vscreen->message(this->getname() + " looks more experienced.");
  }

  if (targetx < 0 || targety < 0) {
    targetx=x;
    targety=y;
  }

  if (!frozen) {
    this->heal();
    this->poison();
    
    if (bleeding >= 0) {
      this->bleed(map);
      if (this->hp < 1) {
	message = this->getname() + " has bled to death.";
	vscreen->message(message);
	return -1;
      }
    }
  }

  if (onfire == 1) {
    this->burn();
    if (this->hp < 1) {
      message = this->getname() + " has burned to death.";
      vscreen->message(message);
      return -1;
    }
    if (frozen == 1) {
      if (mtrand()>0.5) {frozen=1;}
    }
  }

  this->freeze();

  if (this->hp < 1) {
    message = this->getname() + " dies!";
    vscreen->message(message);
    return -1;
  }

  if (time > 0) {time--; return 1;}
  time=(char)( ((40+load/3) / agility) );

  if (numberofspells<0) {this->details();}

  int dx,dy;

  int i,j,lx,ly,hx,hy,targetsighted;

  targetsighted=0;

  Creature * maybetarg;
  active--;

  lx=x-viewdist;
  hx=x+viewdist;
  ly=y-viewdist;
  hy=x+viewdist;

  if (lx<0) {lx=0;}
  if (ly<0) {ly=0;}
  if (hx>map->xsize-1) {hx=map->xsize-1;}
  if (hy>map->ysize-1) {hy=map->ysize-1;}

  int targetrange;

  targetrange=viewdist;

  for (i=-viewdist;i<viewdist;i++) {
    for (j=-viewdist;j<viewdist;j++) {

      if (((j+y)>0 && (j+y) < map->ysize) && ((i+x)>0 && (i+x) < map->xsize)) {

        maybetarg = map->checkmonster(j+y,i+x);
        if (maybetarg != NULL) {
          if ( 1 == maybetarg->isplayer ) {
	    targetsighted = map->cansee(y,x);
	    //targetsighted = lineofsight(map, y, x, j+y, i+x, targetrange);
            if (targetsighted == 1) {active=viewdist;}
            if ( active > 0) {
              targetx = i+x;
              targety = j+y;
              //targetsighted=1;
	      this->redopath(map, vscreen);
//            dx=1;dy=1;
            }
            targetrange = (int)(sqrt((float)((targety-y)*(targety-y)) + (float)((targetx-x)*(targetx-x)))+0.5);
          }
        }
      }
    }
  }

  // If already at target location and not active, walk to a random location.
  if (x==targetx && y == targety && active <= 0 && mtrand()>0.99) {
    targetx=-1;
    targety=-1;
    while (map->cango(targety,targetx)==0 || map->canshoot(targety,targetx)==0) {
      targetx=(int)(map->xsize*mtrand());
      targety=(int)(map->ysize*mtrand());
    }
    this->redopath(map, vscreen);
  }

  dx=0;
  dy=0;

  // Cast a spell, maybe.
  if (targetsighted == 1) {
    if (mp > 0 && (mtrand() > 0.5)) {
      int chooseaspell;
      llist * choosefromspells;
      choosefromspells = *(this->spells());
      chooseaspell = (int)(numberofspells*mtrand());
      while (chooseaspell > 0) {
        choosefromspells=*(&choosefromspells->next);
        chooseaspell--;
      }
      Spell * maybecast;
      maybecast = (Spell *)list_item(&choosefromspells);
      if (mp+1>maybecast->level && targetrange < maybecast->range) {
        this->cast(maybecast,map, vscreen, targety, targetx);
        return 1;
      }
    }

    // Shoot a missile, maybe.
    Item * maybelauncher=NULL;
    if (lhand != NULL) {
      if ((lhand->gettype()).compare("launcher")==0) {
	maybelauncher = lhand;
      }
    }
    if (rhand != NULL) {
      if ((rhand->gettype()).compare("launcher")==0) {
	maybelauncher = rhand;
      }
    }
    if (maybelauncher != NULL && mtrand() > 0.5) {
      Item * missiletolaunch;
      missiletolaunch = this->getmissile(maybelauncher);
      // If we have ammo...
      if (missiletolaunch != NULL) {
	// Fire!
	this->fire(targety, targetx, map, vscreen, 16, missiletolaunch, maybelauncher);
	this->unwield(missiletolaunch);
	this->unwear(missiletolaunch);
	
	return 1;
      }
    }

  }

  dx=0;
  dy=0;
  coords * nextstep;
  if (plannedpath != NULL) {
    nextstep = (coords*)list_item(&plannedpath);

    //    (*nextstep).y=targety;

    if ((*nextstep).x > x) {dx=1;}
    if ((*nextstep).x < x) {dx=-1;}
    if ((*nextstep).y > y) {dy=1;}
    if ((*nextstep).y < y) {dy=-1;}

    if ((x+dx == (*nextstep).x) && (y+dy == (*nextstep).y)) {
      free((coords*)list_item(&plannedpath));
      list_remove(&plannedpath);
    }

  }
  int crushkilldestroy=0;
  maybetarg=map->checkmonster(y+dy,x+dx);
  if (maybetarg != NULL) {
    if (maybetarg->isplayer == 1) { crushkilldestroy=1; }
  }

  if ((map->cango(y+dy,x+dx)==1) && (maybetarg==NULL || crushkilldestroy == 1)) {
    if (active <= 0) {
      for (i=-1;i<2;i++) {
	for (j=-1;j<2;j++) {
	  if ((i != 0 || j != 0) && (i != dx || j != dy)) {
	    if (map->closedoor(y+j,x+i)==1) {
	      time = 2*time;
	      return 1;
	    }
	  }
	}
      }
    }

    this->step(map,dy,dx,vscreen);
  }
  else {
    this->redopath(map, vscreen);
    if (active <= 0 && (targetx != x || targety != y)) {
      targetx=-1;
      targety=-1;
      while (map->cango(targety,targetx)==0 || map->canshoot(targety,targetx)==0) {
	targetx=(int)(map->xsize*mtrand());
	targety=(int)(map->ysize*mtrand());
      }
      this->redopath(map, vscreen);
    }
  }

  return 1;

}

