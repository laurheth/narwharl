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
#include <string>

using namespace std;

extern string knowledge;

Item::Item(Map *map, int sy, int sx, char ssymb, string sname)
{
  color=WHITE;
  name = sname;
  symb = ssymb;
  x = sx;
  y = sy;
  if (x != 0 && y != 0) {
    map->additem(this, y, x);
  }
  spell=NULL;
  uses=0;
  hplost=0;
  mysteryname=name;
  hitbonus=0;
  dmgbonus=0;
  blockchance=0;
  type=0;
  subtype="none";
  mass=1;
  int i;
  for (i=0;i<DMGTYPES;i++) {
    dmg.rolls[i]=0;
    dmg.var[i]=0;
  }

  effects=new char[NUMEFFECTS];

  for (i=0;i<NUMEFFECTS;i++) {
    effects[i]=0;
  }
  stackable = 0;
  consumes=none;
  size = 100;
}

Item::~Item()
{
  delete[] effects;
}

char * Item::geteffects()
{
  return &(effects[0]);
}

char Item::getsymb()
{
  return symb;
}

void Item::setmysteryname(string mystery)
{
  mysteryname=mystery;
}

string Item::getname()
{
  if (knowledge.find(name) != string::npos) {
    return name;
  }
  else {
    return mysteryname;
  }
}

void Item::settype(string newtype)
{
  if (newtype.compare("weapon")==0) {type = 1;symb=')';}
  else if (newtype.compare("ring")==0) {type = 2;symb='=';}
  else if (newtype.compare("armor")==0) {type = 3;symb='[';}
  else if (newtype.compare("amulet")==0) {type = 4;symb='"';}
  else if (newtype.compare("potion")==0) {type = 5;symb='!';stackable=1;}
  else if (newtype.compare("shield")==0) {type = 6;symb=']';}
  else if (newtype.compare("scroll")==0) {type = 7;symb='?';stackable=1;}
  else if (newtype.compare("ring")==0) {type = 8;symb='=';}
  else if (newtype.compare("wand")==0) {type = 9;symb='/';}
  else if (newtype.compare("launcher")==0) {type = 10;symb=')';}
  else if (newtype.compare("missile")==0) {type = 11;symb='(';stackable=1;}
}

string Item::gettype()
{
  switch (type) {
  case 1: return "weapon";
  case 2: return "ring";
  case 3: return "armor";
  case 4: return "amulet";
  case 5: return "potion";
  case 6: return "shield";
  case 7: return "scroll";
  case 8: return "ring";
  case 9: return "wand";
  case 10: return "launcher";
  case 11: return "missile";
  default: return "none";
  }
  return "none";
}

int Item::getmass()
{
  return mass;
}

void Item::setmass(int newmass)
{
  mass = newmass;
}

char Item::getblockchance()
{
  if (type == 6) {
    return blockchance;
  }
  else {
    return 0;
  }
}

void Item::setblockchance(char chance)
{
  blockchance=chance;
}

string Item::getsubcat()
{
  if (subtype.compare("") == 0) {
    return "none";
  }
  else {
    return subtype;
  }
}

void Item::setsubcat(string subcat)
{
  subtype=subcat;
}

void Item::setspell(Spell * newspell)
{
  spell=newspell;
}

string Item::getspellname()
{
  if (spell != NULL) {
    return spell->name;
  }
  return "none";
}

int Item::cast(Creature * reader, Map *map, Screen *vscreen, int targy, int targx)
{
  if (uses > 0) {
    uses--;
    return spell->cast(reader, map, vscreen, targy, targx);
  }
  return 0;
}

void Item::identify()
{
  if (knowledge.find(name) == string::npos) {
    knowledge = knowledge + "," + name;
  }
}

damage Item::getdamage(int take)
{
  if (take == -1) {return dmg;}
  if (take == 0 && (type == 1 || type == 11)) { //Weapon or missile... maybe add another distinction here at a later time
    return dmg;
  }
  else if (take == 1 && (type == 3)) { //armor
    return dmg;
  }
  else {
    damage calcdmg;
    int hurts;
    hurts = mass/10;
    if (hurts == 0) {hurts=1;}
    int i;
    for (i=0;i<DMGTYPES;i++) {
      calcdmg.rolls[i]=0;
      calcdmg.var[i]=0;
    }
    calcdmg.rolls[0] = 1;
    calcdmg.var[0] = hurts;
    return calcdmg;
  }
}

void Item::setdamage(damage newdamage)
{
  dmg = newdamage;
}

dmgval sumdmg(dmgval val1, dmgval val2, int scale)
{
  int i;
  for (i=0;i<DMGTYPES;i++) {
    val1.dv[i] = val1.dv[i] + scale*val2.dv[i];
    if (val1.dv[i] < 0) {val1.dv[i] = 0;}
  }
  return val1;
}

dmgval calcdamage(damage wpndmg, int strength, int dudesize)
{
  dmgval value;
  int i,j;
  int notzero[DMGTYPES];
  int strbonus = ((dudesize * strength)/100)-10;//(dudesize * (strength-10))/100;
  for (i=0;i<DMGTYPES;i++) {
    notzero[i] = 0;
    value.dv[i]=0;
    for (j=0;j<wpndmg.rolls[i];j++) {
      notzero[i]=1;
      value.dv[i] = value.dv[i] + (int)(mtrand() * wpndmg.var[i]) + 1;
    }
  }
  if (notzero[0] == 1 && notzero[1] == 1) {
    value.dv[0] = value.dv[0] + strbonus/2;
    value.dv[1] = value.dv[1] + strbonus/2;
    if (value.dv[0] < 1 && (value.dv[1] < 1)) {value.dv[1]=1;}
  }
  else if (notzero[0] == 1 && notzero[1] == 0) {
    value.dv[0] = value.dv[0] + strbonus;
    if (value.dv[0] < 1) {value.dv[0]=1;}
  }
  else if (notzero[0] == 0 && notzero[1] == 1) {
    value.dv[1] = value.dv[1] + strbonus;
    if (value.dv[1] < 1) {value.dv[1]=1;}
  }

  for (i=2;i<DMGTYPES;i++) {
    if (value.dv[i]<0 && notzero[i] == 1) {value.dv[i] = 1;}
  }

  return value;

}

dmgval armorcalc(llist **armor, bpart where)
{
  damage armorprot;
  llist * am = *armor;
  dmgval protection;
  int i;
  int acceptable;
  Item * item;
  for (i=0;i<DMGTYPES;i++) {
    protection.dv[i] = 0;
  }

  if (am != NULL) {
    while (am != NULL) {
      acceptable=0;
      item = ((Item *)list_item(&am));
      if (where == all) {acceptable = 1;}
      if ((item->getsubcat()).compare("cloak")==0) {
	if (where == upper || where == lower) {acceptable = 1;}
      }
      if ((item->getsubcat()).compare("upper body")==0) {
        if (where == upper) {acceptable = 1;}
      }
      if ((item->getsubcat()).compare("lower body")==0) {
        if (where == lower) {acceptable = 1;}
      }
      if ((item->getsubcat()).compare("gauntlets")==0) {
        if (where == lhand || where == rhand) {acceptable = 1;}
      }
      if ((item->getsubcat()).compare("boots")==0) {
        if (where == feet) {acceptable = 1;}
      }
      if ((item->getsubcat()).compare("helmet")==0) {
        if (where == head) {acceptable = 1;}
      }


      if (acceptable == 1) {
        protection = sumdmg(protection,calcdamage(item->getdamage(1),10,100),1);
      }
      am = *(&am->next);
    }
  }

  return protection;
  
}

bpart whereonbody(int attackersize, int defendersize, string * bpartname)
{

  bpart wherehit;
  string nameofpart;
  int vertical=(int)(1000 * mtrand());
  vertical = vertical * attackersize/defendersize;
  int horizontal=(int)(10 * mtrand()+1);
  if (vertical > 950 && horizontal < 3) { wherehit = head; }
  else if ((vertical > 600 && vertical < 800) && (horizontal < 3) ) { wherehit = lhand; }
  else if ((vertical > 600 && vertical < 800) && (horizontal < 5) ) { wherehit = rhand; }
  else if (vertical > 500) { wherehit = upper; }
  else if (vertical > 50 ) { wherehit = lower; }
  else {wherehit = feet;}

  switch (wherehit) {
  case head: nameofpart="head"; break;
  case lhand: nameofpart="left hand"; break;
  case rhand: nameofpart="right hand"; break;
  case upper: nameofpart="upper body"; break;
  case lower: nameofpart="lower body"; break;
  case feet: nameofpart="feet"; break;
  }

  if (bpartname != NULL) {
    *bpartname = *bpartname + nameofpart + " ";//; + ", ";
  }

  return wherehit;

}
