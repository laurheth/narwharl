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
#include <cmath>

#ifndef WIN
#include <unistd.h>
#define SLEEP(a) usleep(a*1000)
#else
#include <windows.h>
#define SLEEP(a) Sleep(a)
#endif

using namespace std;

// Constructor
Spell::Spell()
{
  effect=noeffect;
  travel=untilopaque;
  trail=1;
  range=0;
  skiprange=1;
  spread=0;
  blastradius=0;
  stopattarget=0;
  color=WHITE;
  int ii;
  for (ii=0;ii<6;ii++) {
    args[ii]=0;
  }
}

// Destructor
Spell::~Spell()
{
}


// Cast spell.
int Spell::cast(Creature * caster, Map * map, Screen * vscreen, int targy, int targx)
{
  int spellcasting=caster->getskill("magic");
  int y,x, ty,tx; // initial y and x, target y and x
  y = caster->y; // Caster position for y and x
  x = caster->x;
  string message; // "Caster casts spell." Make this better later.
  message = caster->getname();
  message = message + " casts ";
  message = message + this->name;
  ty=y;
  tx=x;
  // If range is greater than zero (self), pick a target.
  if (this->range > 0) {
    // If caster is a monster, don't ask.
    if (caster->isplayer == 1) {
      vscreen->movecursor(map, &ty,&tx);
    }
    else {
      ty=targy;
      tx=targx;
    }
  }
  // Don't calculate travel if self is targetted.
  if (ty != y || tx != x) {
    int rangeleft, targdist;
    double startangle;
    double spreadangle;
    int spreadx=0;
    int spready=0;
    int placeholder=0;
    // If there is spread, determine spreadangle and startangle.
    if (this->spread >= 1) {
      spreadangle=atan2((double)(this->spread),(double)(this->range))/(2*(double)(this->spread));
      startangle=atan2((double)(ty-y),(double)(tx-x));
    }
    rangeleft=this->range;
    targdist = (ty-y)*(ty-y) + (tx-x)*(tx-x);
    targdist = (int)(sqrt((double)targdist)+0.5);
    if (targdist < rangeleft && this->stopattarget==1) {rangeleft=targdist;}

    if (this->spread >= 1) {
      int iii, spreadreps, initspread, spreadrange;

      spreadreps = 2*(this->spread);
      for (iii=-spreadreps;iii<spreadreps+1;iii++) {
	float ttx, tty;
	ttx = (float)x + (this->range)*(cos((float)(startangle+iii*spreadangle)));
	tty = (float)y + (this->range)*(sin((float)(startangle+iii*spreadangle)));
	spreadrange=rangeleft;
	switch (this->travel) {
          // if you can see it, you can hit it.
        case untilopaque:
          los(map, y, x, &tty, &ttx, &spreadrange, spellsee, this->skiprange);
          break;
          // Travels until a collision.
        case untilhit:
          los(map, y, x, &tty, &ttx, &spreadrange, spellhit, this->skiprange);
          break;
        }
      }
      

    }

    switch (this->travel) {
      // if you can see it, you can hit it.
    case untilopaque:
      los(map, y, x, &ty, &tx, &rangeleft, spellsee, this->skiprange);
      break;
      // Travels until a collision.
    case untilhit:
      los(map, y, x, &ty, &tx, &rangeleft, spellhit, this->skiprange);
      break;
    }
  }
  else {
    // If the self cannot be targetted, return -1 for "Nevermind."
    if (this->skiprange > 0) {return -1;}
  }
  map->seteffected(ty,tx,1); // Set resulting target tile to effected.
  if (this->skiprange > 0) {map->seteffected(y,x,0);}
  // Take into account blast radius, if there is one.
  if (this->blastradius > 0) {
    aoe(map, ty, tx, this->blastradius, spellsee);
  }

  // All effected tiles are now labelled as such! Now to apply the effect everywhere.
  damage spelldmg;
  int i,j,k,lx,ly,hx,hy,effrange, damage;
  dmgval armor, spdmg, dmgdone;
  Creature * creature;
  for (i=0;i<DMGTYPES;i++) {
    spelldmg.rolls[i]=0;
    spelldmg.var[i]=0;
  }
  
  switch(this->effect) {
  case firedmg:
    spelldmg.rolls[2]=this->args[0];
    spelldmg.var[2]=this->args[1]+spellcasting;
    break;
  case icedmg:
    spelldmg.rolls[3]=this->args[0];
    spelldmg.var[3]=this->args[1]+spellcasting;
    break;
    }

  effrange = this->range + this->blastradius + 3;
  lx = x-effrange;
  ly = y-effrange;
  hx = x+effrange;
  hy = y+effrange;
  if (lx < 0) {lx=0;}
  if (ly < 0) {ly=0;}
  if (hx > map->xsize-1) {hx=map->xsize-1;}
  if (hy > map->ysize-1) {hy=map->ysize-1;}
  int angle=0;
  int distout=0;
  for (distout=0; distout<effrange;distout++) {
    for (angle=0; angle<12*effrange;angle++) {
      i=x+(int)(((float)distout)*cos(2*PI*((float)angle)/((float)effrange*12))+0.5);
      j=y+(int)(((float)distout)*sin(2*PI*((float)angle)/((float)effrange*12))+0.5);
      if ((i < 0 || j < 0) || (i > map->xsize-1 || j > map->ysize-1)) {i=x;j=y;}
      //  for (i=lx;i<hx+1;i++) {
      //    for (j=ly;j<hy+1;j++) {
      if (map->iseffected(j,i) != 0) {

	if (map->cansee(j,i) == 1) {vscreen->drawatpos(j,i,'*',this->color);}

	creature = map->checkmonster(j,i);

	if (creature != NULL) {
	  if (this->effect == firedmg || this->effect == icedmg) {

	    armor = armorcalc(creature->armor(),all);
	    dmgdone = sumdmg(calcdamage(spelldmg,10,100),armor,-1);
	    damage = 0;
	    for (k=0;k<DMGTYPES;k++) {
	      damage = damage + dmgdone.dv[k];
	      if (k==1 && dmgdone.dv[k] > 0) {
		int wutev=dmgdone.dv[k];
		while (wutev > 0) {
		  wutev = wutev-10;
		  map->makebloody(creature->y+(int)(3*mtrand())-1,creature->x+(int)(3*mtrand())-1);
		}
	      }
	    }
	    if (damage > 0) {
	      creature->attacker = caster->id;
	    }
	    creature->hp = creature->hp - damage;
	    if ((int)(mtrand() * (creature->hp )) + 1 < (int)(mtrand() * damage)) {
	      if (this->effect == firedmg) {
		creature->onfire = 1;
	      }
	      if (this->effect == icedmg) {
		creature->frozen = 1;
	      }
	    }
	  }
	  else if (this->effect == identifyitem && creature->isplayer != 0) {
	    Item * item;
	    item = vscreen->inventory(creature, "Use on item : ", "all");
	    if (item != NULL) {
	      item->identify();
	      return 1;
	    }
	    else {
	      return 0;
	    }
	  }
	}
	//map->makebloody(j,i);
	
	map->seteffected(j,i,0);
      }
    }
    //    usleep(20000);
    SLEEP(20);
  }

  vscreen->message(message);

  return 1;
}
