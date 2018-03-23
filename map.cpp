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
#include <fstream>
#include "nheader.h"
//#include <ncurses/cursesapp.h>
#include <cstring>
#include <string>

using namespace std;

extern string knowledge;

extern Stuffgetter * objget;

Map::Map(int sseed)
{
  currentmon=NULL;
  seed = sseed;
  dlvl=1;
  maxdlvl=1;
  plvl=1;
  pop=0;
  maxpop=0;
}

Map::~Map()
{
  //  this->savemap();
}

void Map::setmaxdlvl(char newmax)
{
  if (newmax > maxdlvl) {
    maxdlvl = newmax;
  }
}

void Map::addnewmonster()
{
  int i,j,k;
  k=11;
  while (k>0) {
    k--;
    i=(int)(xsize * mtrand())+1;
    j=(int)(ysize * mtrand())+1;
    if (this->cango(j,i) == 1 && this->canshoot(j,i) == 1) {
      if (this->cansee(j,i) == 0) {
	if (this->checkmonster(j,i) == NULL) {
	  k=-1;
	  placemonster(j,i);
	}
      }
    }
  }
  //  if (k == -1) {pop++;}
}

int Map::loop(Screen * vscreen)
{
  int returned;
  if (currentmon==NULL) {
    currentmon=*mlist;
  }
  Creature * actor;
  actor = (Creature*)list_item(&currentmon);

  if (actor == NULL) {
    list_remove(&currentmon);
    return 1;
  }

  returned = actor->ai(this,vscreen);
  if (currentmon == NULL) {
    return returned;
  }
  currentmon = *(&currentmon->next);

  if (returned == (-1)) {
    actor->die(this);
    llist ** toremove = list_search(mlist, (void*)actor);
    if (list_item(toremove) == (void*)actor) {
      list_remove(toremove);
      delete actor;
      actor=NULL;
      returned = 1;
    }
  }

  int numtries;
  numtries=2*(maxpop-pop);
  if (pop < maxpop) {
    while (pop < maxpop && numtries>0) {
      this->addnewmonster();
      numtries--;
    }
  }

  if (returned != 1) {
    if (returned == 2) {
      this->savemap();
    }
    else {
      this->unsave();
    }
  }

  return returned;

}

void Map::changelevel(char change, Creature * changer)
{
  int cx, cy;
  char tolvl;
  tolvl=dlvl+change;
  if (tolvl == 0) {return;}
  cx = changer->x;
  cy = changer->y;
  this->leave(cy,cx);
  llist ** removeactor;
  removeactor = list_search(mlist, (void*)changer);
  if (list_item(removeactor) == (void*)changer) {
    list_remove(removeactor);
  }
  this->savemap(dlvl);
  //  this->deallocatemap();

  this->setmaxdlvl(tolvl);
  this->dlvl=tolvl;

  if (this->loadmap(mlist,tolvl) == 0) {
    this->newgenmap(mlist, &cy, &cx, this);
  }
  plvl = changer->level;
  list_add(mlist, (void *)changer);
  currentmon=NULL;
  this->go(changer, cy, cx);
  this->savemap();
}

void Map::allocatemap(int y, int x)
{

  int i,j;

  if (y!=0 && x!= 0) {
    ysize=y;
    xsize=x;
  }
  else {
    ysize = 20;//+(int)(101*mtrand());
    xsize = 80;//+(int)(101*mtrand());
  }

  xsize=50;
  ysize=50;

  map = new tile*[ysize];
  for (i=0;i<ysize;i++) {
    map[i] = new tile[xsize];
  }

  plan=NULL;

  for (i=0;i<xsize;i++)
    {
      for (j=0;j<ysize;j++)
	{
	  map[j][i].passable = 0;
	  map[j][i].symbol = '#';
	  map[j][i].seen = 0;
	  map[j][i].visible = 0;
	  map[j][i].seethrough = 0;
	  map[j][i].creature = NULL;
	  map[j][i].ilist = NULL;
	  map[j][i].color = WHITE;
	  map[j][i].seensymbol = ' ';
	  map[j][i].effected=0;
	  map[j][i].shootthrough=0;
	}
    }

}

void Map::deallocatemap() {
  int i,j;

  Creature * montorem;
  Item * itemtorem;
  llist * templist;

  
  for (i=0;i<xsize;i++) {
    for (j=0;j<ysize;j++) {
      if (map[j][i].ilist != NULL) {
	while (map[j][i].ilist != NULL) {
	  itemtorem = (Item *)list_item(&(map[j][i].ilist));
	  list_remove(&(map[j][i].ilist));
	  if (itemtorem != NULL) {
	    delete itemtorem;
	  }
	}
      }
    }
  }

  templist=*mlist;
  while (templist != NULL) {
    montorem = (Creature *)list_item(&templist);
    delete montorem;
    list_remove(&templist);
  }
  *mlist = NULL;

  for (i=0;i<xsize;i++) {
    delete[] map[i];
  }
  delete[] map;
  map=NULL;
}

int Map::inside(int y, int x)
{
  if ((y>=0 && x>=0) && (y<ysize && x<xsize)) {return 1;}
  return 0;
}

int Map::cango(int y, int x)
{
  if (this->inside(y,x)==0) {
    return 0;
  }
  return map[y][x].passable;
}

int Map::canshoot(int y, int x)
{
  if (this->inside(y,x)==0) {
    return 0;
  }
  return map[y][x].shootthrough;
}

int Map::cansee(int y, int x)
{
  if (this->inside(y,x)==0) {
    return 0;
  }  
  return map[y][x].visible;
}

int Map::seen(int y, int x)
{
  if (this->inside(y,x)==0) {
    return 0;
  }
  return map[y][x].seen;
}

int Map::iseffected(int y, int x)
{
  return map[y][x].effected;
}

void Map::seteffected(int y, int x, int value)
{
  map[y][x].effected=value;
}

int Map::getopacity(int y, int x)
{
  return map[y][x].opacity;
}

llist ** Map::getitems(int y, int x)
{
  return &(map[y][x].ilist);
}

char Map::getfloorsymb(int y, int x)
{
  return map[y][x].symbol;
}

int Map::opendoor(int y, int x)
{
  if (map[y][x].symbol == '+') {

    map[y][x].symbol = '-';
    map[y][x].shootthrough = 1;
    map[y][x].seethrough = 1;
  }
  else if (map[y][x].symbol == '-') {
    return 2;
  }
  else {
    return 0;
  }
  return 1;
}

int Map::closedoor(int y, int x)
{
  if (map[y][x].symbol == '-') {

    if (map[y][x].creature != NULL || map[y][x].ilist != NULL) {
      return 3;
    }


    map[y][x].symbol = '+';
    map[y][x].shootthrough = 0;
    map[y][x].seethrough = 0;
  }
  else if (map[y][x].symbol == '+') {
    return 2;
  }
  else {
    return 0;
  }
  return 1;
}

int Map::getsymb(int y, int x)
{
  unsigned long int symb;
  int color=1;

  if (map[y][x].visible == 1) {
    if (map[y][x].creature == NULL) {
      if (list_item(&(map[y][x].ilist)) == NULL) {
	symb = (map[y][x].symbol);
	if (map[y][x].bloody ==1) {
	  color = RED;
	}
	else {color = (map[y][x].color);}
      }
      else {
	symb = ((Item *)list_item(&(map[y][x].ilist)))->getsymb();
	color = ((Item *)list_item(&(map[y][x].ilist)))->color;
      }
    }
    else {
      Creature *creature;
      creature = map[y][x].creature;
      symb = creature->getsymb();
      color = creature->getcolor();
    }
    if ((char)symb != '@') {
      map[y][x].seensymbol = (char)symb;
    }
  }
  else {
    if (this->inside(y,x)==0) {
      symb = ' '; // | COLOR_PAIR(1);
      color = 1;
    }
    else {
      symb = (map[y][x].seensymbol); // | COLOR_PAIR(1);
      color=1;
    }
  }
  if (color>0) {
    return symb | COLOR_PAIR(color);
  }
  else {
    return symb | COLOR_PAIR(-color) | A_BOLD;
  }
}

string Map::look(int y, int x)
{
  string str="> ";
  string bloody="";
  if (this->inside(y,x) == 0) {
    str = str + "You can't see that area.";
    return str;    
  }
  if (this->cansee(y,x) == 0) {

    str = str + "You can't see that area.";
    return str;

  }
  if (map[y][x].creature != NULL) {
    str = str + "A ";
    str = str + (map[y][x].creature)->getname();
    str = str + " on ";
  }
  if (list_item(&(map[y][x].ilist)) != NULL) {
    str = str + "a ";
    str = str + ((Item *)list_item(&(map[y][x].ilist)))->getname();
    if (list_item(&(map[y][x].ilist)->next) != NULL) {
      str = str + " and other stuff";
    }
    str = str + " on ";
  }
  if (map[y][x].bloody==1) {bloody="blood covered ";}
  switch (map[y][x].symbol) {
  case '.':
    str = str + bloody + "floor.";
    break;
  case '#':
    str = str + "a " + bloody + "wall.";
    break;
  default:
    str = str + "empty space.";
  }
  return str;
}

void Map::additem(Item * item, int y, int x)
{
  list_add(&(map[y][x].ilist), (void *) item);
}

Creature * Map::checkmonster(int y, int x)
{
  return map[y][x].creature;
}

int Map::seethrough(int y, int x)
{
  return map[y][x].seethrough;
}

void Map::see(int y, int x)
{
  map[y][x].visible = 1;
  map[y][x].seen = 1;
}

void Map::unsee(int y, int x)
{
  map[y][x].visible = 0;
}

void Map::go(Creature *creature, int y, int x)
{
  map[y][x].creature = creature;
}

void Map::makebloody(int y, int x)
{
  map[y][x].bloody=1;
}

void Map::put(Item *item, int y, int x)
{
  list_add(this->getitems(y,x),(void *)item);
}

void Map::leave(int y, int x)
{
  map[y][x].creature = NULL;
}


// Load function
int Map::loadmap(llist ** mmlist)
{
  return this->loadmap(mmlist,0);
}

int Map::loadmap(llist ** mmlist, int lvl)
{
  string mapto, creaturefile, itemfile;
  mapto = "save/map.m";
  creaturefile = "save/creatures.m";
  itemfile = "save/items.m";

  if (lvl > 0) {
    char tch='a';
    tch += (char)(lvl-1);
    mapto += tch;
    creaturefile += tch;
    itemfile += tch;
  }

  ifstream file1 (mapto.c_str());
  ifstream file2 (creaturefile.c_str());
  ifstream file3 (itemfile.c_str());
  ifstream file4 ("save/itemknowledge.txt");

  int i,j;
  pop=0;
  maxpop=0;

  if ( !file1.is_open() || !file2.is_open() || !file3.is_open() ) {return 0;}
  if (map != NULL) {this->deallocatemap();}

  if (file4.is_open()) {
    getline(file4, knowledge);

    for (i=0;i<(objget->numpots);i++) {
      getline(file4,objget->potmystery[i]);
    }

    for (i=0;i<(objget->numspells);i++) {
      getline(file4,objget->wandmystery[i]);
    }

    for (i=0;i<(objget->numspells);i++) {
      getline(file4,objget->scrollmystery[i]);
    }

    file4.close();
  }

  //  return 0;
  char line[128];

  file1.getline(line,128);
  seed = atoi (line);
  file1.getline(line,128);
  ysize = atoi (line);
  file1.getline(line,128);
  xsize = atoi (line);
  file1.getline(line,128);
  dlvl = atoi (line);
  file1.getline(line,128);
  this->setmaxdlvl((char)(atoi(line)));

  this->allocatemap(ysize, xsize);

  for (j=0;j<ysize;j++) {
    for (i=0;i<xsize;i++) {
      file1.getline(line,128,'@');
      map[j][i].symbol = line[0];
      map[j][i].visible = line[1];
      map[j][i].passable = line[2];
      map[j][i].seen = line[3];
      map[j][i].bloody = line[4];
      map[j][i].seethrough = line[5];
      map[j][i].opacity = line[6];
      map[j][i].color = atoi(&(line[7]));
      map[j][i].seensymbol = line[8];
      map[j][i].shootthrough = line[9];
    }
  }

  file1.close();

  int monid=0;
  int playerlevel=1;
  char newskills[NUMSKILLS];
  string name;
  string mysteryname="none";
  char symb;
  int isplayer;
  int itemsize=100;
  int x,y,k;
  int uses=0;
  int viewdist;
  int isitem=0;
  int experience=0;
  int strength,dexterity,agility,toughness,size,level,maxhp,hp,poison,healcount,healrate,bleeding;
  int time=0;
  int hplost=0;
  i_fuel consumes=none;
  poison=0;
  bleeding=0;
  healcount=200;
  healrate=10;
  int blockchance=0;
  int hitbonus=0;
  int dmgbonus=0;
  int maxmp=0;
  int mp=0;
  int onfire=0;
  int frozen=0;
  int special=0;
  int rechargerate=10;
  int rechargecount=100;
  int targetx=-1;
  int targety=-1;
  int active=-1;
  string temp;
  string itemtype;
  Spell * spellcastbyitem=NULL;
  int mass;
  int color;

  natwep * naturalweapons;
  naturalweapons=NULL;
  int natwepnums=0;

  string subcat;
  string spellknown;
  llist * newspelllist=NULL;
  damage setdmg;
  char * effpointer; // pointer to effects array in items.
  char effects[NUMEFFECTS]; // effects get read into this array.
  for (k=0;k<NUMEFFECTS;k++) {
    effects[k]=0;
  }
  mass =1;
  itemtype="none";
  subcat = "none";
  for (k=0;k<DMGTYPES;k++){
    setdmg.rolls[k] = 0;
    setdmg.var[k] = 0;
  }
  for (k=0;k<NUMSKILLS;k++) {
    newskills[k];
  }
  string worn;
  worn =" ";

  while(!file2.eof()) {

    file2.getline(line,128);
    char * tok;
    tok = strtok (line," ");
    if (tok != NULL) {
      if (isitem==1) {
	if (strcmp(tok,"[name]") == 0) {
	  tok = strtok (NULL,"\n");
	  name = tok;
	}
	else if (strcmp(tok,"[mysteryname]") == 0) {
	  tok = strtok (NULL,"\n");
	  mysteryname = tok;
	}
	else if (strcmp(tok,"[spell]") == 0) {
	  tok = strtok (NULL,"\n");
	  temp = tok;
	  spellcastbyitem = objget->spellbyname(temp);
	}
	else if (strcmp(tok,"[symb]") == 0) {
	  tok = strtok (NULL," ");
	  symb = *tok;
	}
	else if (strcmp(tok,"[lhand]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[rhand]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[lring]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[rring]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[neck]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[worn]") == 0) {
	  worn = tok;
	}
	else if (strcmp(tok,"[type]") == 0) {
	  tok = strtok (NULL," ");
	  itemtype = tok;
	}
	else if (strcmp(tok,"[mass]") == 0) {
	  tok = strtok (NULL," ");
	  mass = atoi(tok);
	}
        else if (strcmp(tok,"[size]") == 0) {
          tok = strtok (NULL," ");
          itemsize = atoi(tok);
        }
	else if (strcmp(tok,"[hitbonus]") == 0) {
	  tok = strtok (NULL," ");
	  hitbonus = atoi(tok);
	}
	else if (strcmp(tok,"[dmgbonus]") == 0) {
	  tok = strtok (NULL," ");
	  dmgbonus = atoi(tok);
	}
	else if (strcmp(tok,"[fuel]") == 0) {
	  tok = strtok (NULL," ");
	  consumes = (i_fuel)(atoi(tok));
	}
	else if (strcmp(tok,"[hplost]") == 0) {
	  tok = strtok (NULL," ");
	  hplost = atoi(tok);
	}
	else if (strcmp(tok,"[uses]") == 0) {
	  tok = strtok (NULL," ");
	  uses = atoi(tok);
	}
	else if (strcmp(tok,"[blockchance]") == 0) {
	  tok = strtok (NULL," ");
	  blockchance = atoi(tok);
	}
	else if (strcmp(tok,"[color]") == 0) {
	  tok = strtok (NULL," ");
	  color = atoi(tok);
	}
	else if (strcmp(tok,"[subtype]") == 0) {
	  tok = strtok (NULL," ");
	  subcat = tok;
	}
	else if (strcmp(tok,"[effects]") == 0) {
	  for (k=0;k<NUMEFFECTS;k++) {
	    tok = strtok (NULL," ");
	    effects[k] = atoi(tok);
	  }
	}
	else if (strcmp(tok,"[dmgroll]") == 0) {
	  for (k=0;k<DMGTYPES;k++) {
	    tok = strtok (NULL," ");
	    setdmg.rolls[k] = atoi(tok);
	  }
	}
	else if (strcmp(tok,"[dmgvar]") == 0) {
	  for (k=0;k<DMGTYPES;k++) {
	    tok = strtok (NULL," ");
	    setdmg.var[k] = atoi(tok);
	  }
	}
	else if (strcmp(tok,"[enditem]") == 0) {
	  isitem = 0;
	  Item *item = new Item(this, 0, 0, symb, name);
	  list_append(map[y][x].creature->inventory(),(void *)item);
	  if (worn.compare("[lhand]") == 0 || worn.compare("[rhand]") == 0) {
	    map[y][x].creature->wield(item);
	  }
	  if (worn.compare("[lring]") == 0 || worn.compare("[rring]") == 0) {
	    map[y][x].creature->wear(item);
	  }
	  if (worn.compare("[worn]") == 0) {
	    list_add(map[y][x].creature->armor(),(void *)item);
	  }
	  if (mysteryname.compare("none") != 0) {
	    item->mysteryname = mysteryname;
	  }
	  effpointer = item->geteffects();
	  for (k=0;k<NUMEFFECTS;k++) {
	    *effpointer++ = effects[k];
	  }
	  item->setspell(spellcastbyitem);
	  spellcastbyitem=NULL;
	  item->hplost=hplost;
	  hplost=0;
	  item->settype(itemtype);
	  item->setsubcat(subcat);
	  item->setmass(mass);
	  item->setdamage(setdmg);
	  item->color = color;
	  item->uses=uses;
	  item->hitbonus=(char)hitbonus;
	  item->dmgbonus=(char)dmgbonus;
	  item->size = itemsize;
	  item->consumes = consumes;
	  consumes=none;
	  hitbonus=0;
	  dmgbonus=0;
	  uses=0;
	  item->setblockchance(((char)blockchance));
	  item=NULL;
	  worn="asdf";
	  mass =1;
	  blockchance=0;
	  itemsize=100;
	  itemtype="none";
	  subcat = "none";
	  mysteryname="none";
	  for (k=0;k<NUMEFFECTS;k++) {
	    effects[k]=0;
	  }
	  for (k=0;k<DMGTYPES;k++){
	    setdmg.rolls[k] = 0;
	    setdmg.var[k] = 0;
	  }
	}
      }
      else if (strcmp(tok,"[name]") == 0) {
	tok = strtok (NULL,"\n");
	name = tok;
      }
      else if (strcmp(tok,"[symb]") == 0) {
	tok = strtok (NULL," ");
	symb = *tok;
      }
      else if (strcmp(tok,"[x,y]") == 0) {
	tok = strtok (NULL,",");
	x = atoi(tok);
	tok = strtok (NULL,",");
	y = atoi(tok);
      }
      else if (strcmp(tok,"[viewdist]") == 0) {
	tok = strtok (NULL," ");
	viewdist = atoi(tok);
      }
      else if (strcmp(tok,"[strength]") == 0) {
	tok = strtok (NULL," ");
	strength = atoi(tok);
      }
      else if (strcmp(tok,"[dexterity]") == 0) {
	tok = strtok (NULL," ");
	dexterity = atoi(tok);
      }
      else if (strcmp(tok,"[agility]") == 0) {
	tok = strtok (NULL," ");
	agility = atoi(tok);
      }
      else if (strcmp(tok,"[skills]") == 0) {
	for (k=0;k<NUMSKILLS;k++) {
	  tok = strtok (NULL," ");
	  newskills[k] = (char)(atoi(tok));
	}
      }
      else if (strcmp(tok,"[toughness]") == 0) {
	tok = strtok (NULL," ");
	toughness = atoi(tok);
      }
      else if (strcmp(tok,"[size]") == 0) {
	tok = strtok (NULL," ");
	size = atoi(tok);
      }
      else if (strcmp(tok,"[level]") == 0) {
	tok = strtok (NULL," ");
	level = atoi(tok);
      }
      else if (strcmp(tok,"[experience]") == 0) {
	tok = strtok (NULL," ");
	experience = atoi(tok);
      }
      else if (strcmp(tok,"[time]") == 0) {
	tok = strtok (NULL," ");
	time = atoi(tok);
      }
      else if (strcmp(tok,"[maxhp]") == 0) {
	tok = strtok (NULL," ");
	maxhp = atoi(tok);
      }
      else if (strcmp(tok,"[hp]") == 0) {
	tok = strtok (NULL," ");
	hp = atoi(tok);
      }
      else if (strcmp(tok,"[poison]") == 0) {
	tok = strtok (NULL," ");
	poison = atoi(tok);
      }
      else if (strcmp(tok,"[bleeding]") == 0) {
        tok = strtok (NULL," ");
        bleeding = atoi(tok);
      }
      else if (strcmp(tok,"[healrate]") == 0) {
	tok = strtok (NULL," ");
	healrate = atoi(tok);
      }
      else if (strcmp(tok,"[healcount]") == 0) {
	tok = strtok (NULL," ");
	healcount = atoi(tok);
      }
      else if (strcmp(tok,"[maxmp]") == 0) {
	tok = strtok (NULL," ");
	maxmp = atoi(tok);
      }
      else if (strcmp(tok,"[mp]") == 0) {
	tok = strtok (NULL," ");
	mp = atoi(tok);
      }
      else if (strcmp(tok,"[rechargerate]") == 0) {
	tok = strtok (NULL," ");
	rechargerate = atoi(tok);
      }
      else if (strcmp(tok,"[rechargecount]") == 0) {
	tok = strtok (NULL," ");
	rechargecount = atoi(tok);
      }
      else if (strcmp(tok,"[color]") == 0) {
	tok = strtok (NULL," ");
	color = atoi(tok);
      }
      else if (strcmp(tok,"[isplayer]") == 0) {
	tok = strtok (NULL," ");
	isplayer = atoi(tok);
      }

      else if (strcmp(tok,"[onfire]") == 0) {
	onfire = 1;
      }
      else if (strcmp(tok,"[frozen]") == 0) {
	frozen = 1;
      }
      else if (strcmp(tok,"[special]") == 0) {
	special = 1;
      }


      else if (strcmp(tok,"[spell]") == 0) {
	tok = strtok (NULL,"\n");
	spellknown=tok;
	list_append(&newspelllist, (void*)(objget->spellbyname(spellknown)));
      }

      else if (strcmp(tok,"[target]") == 0) {
	tok = strtok (NULL," ");
        targety = atoi(tok);
	tok = strtok (NULL," ");
        targetx = atoi(tok);
      }
      else if (strcmp(tok,"[active]") == 0) {
        tok = strtok (NULL," ");
        color = atoi(tok);
      }

      else if (strcmp(tok,"[bite]") == 0) {
        natwepnums++;
        naturalweapons=(natwep*)realloc(naturalweapons, natwepnums * sizeof(natwep));
        naturalweapons[natwepnums-1] = bite;
      }

      else if (strcmp(tok,"[claw]") == 0) {
        natwepnums++;
        naturalweapons=(natwep*)realloc(naturalweapons, natwepnums * sizeof(natwep));
        naturalweapons[natwepnums-1] = claw;
      }
      else if (strcmp(tok,"[slam]") == 0) {
        natwepnums++;
        naturalweapons=(natwep*)realloc(naturalweapons, natwepnums * sizeof(natwep));
        naturalweapons[natwepnums-1] = slam;
      }
      else if (strcmp(tok,"[horns]") == 0) {
        natwepnums++;
        naturalweapons=(natwep*)realloc(naturalweapons, natwepnums * sizeof(natwep));
        naturalweapons[natwepnums-1] = horns;
      }


      else if (strcmp(tok,"[item]") == 0) {
	isitem = 1;
      }
      else if (strcmp(tok,"[end]") == 0) {
	if (isplayer == 1) {
	  map[y][x].creature = new Player(this, y, x, symb, name);
	  this->plvl = level;
	}
	else {
	  map[y][x].creature = new Monster(this, y, x, symb, name);
	  if (targety != -1 && targetx != -1) {
	    ((Monster*)(map[y][x].creature))->targetx=targetx;
            ((Monster*)(map[y][x].creature))->targety=targety;
	    targetx=-1;
	    targety=-1;
	  }
	  if (active != -1) {
            ((Monster*)(map[y][x].creature))->active=active;
	    active=-1;
	  }
	  pop++;
	  maxpop++;
	}
	list_add(mmlist, (void *) map[y][x].creature);
	list_merge((map[y][x].creature)->spells(),newspelllist);
	newspelllist=NULL;
	for (k=0;k<NUMSKILLS;k++) {
	  (map[y][x].creature)->skills[k]=newskills[k];
	  newskills[k]=0;
	}
	(map[y][x].creature)->strength=strength;
	(map[y][x].creature)->dexterity=dexterity;
	(map[y][x].creature)->agility=agility;
	(map[y][x].creature)->toughness=toughness;
	(map[y][x].creature)->size=size;
	(map[y][x].creature)->level=level;
	(map[y][x].creature)->experience=experience;
	(map[y][x].creature)->maxhp=maxhp;
	(map[y][x].creature)->hp=hp;
        (map[y][x].creature)->natwepnum=natwepnums;
        (map[y][x].creature)->naturalweapons=naturalweapons;
	(map[y][x].creature)->color=color;
	(map[y][x].creature)->poisoned=poison;
        (map[y][x].creature)->bleeding=bleeding;
	(map[y][x].creature)->healrate=healrate;
	(map[y][x].creature)->maxmp=maxmp;
	(map[y][x].creature)->mp=mp;
	(map[y][x].creature)->rechargerate=rechargerate;
	(map[y][x].creature)->rechargecount=rechargecount;
	(map[y][x].creature)->healcount=healcount;
	(map[y][x].creature)->time=(char)time;
	(map[y][x].creature)->onfire=(bool)onfire;
	(map[y][x].creature)->frozen=(bool)frozen;
	(map[y][x].creature)->special=(bool)special;

	natwepnums=0;
	experience=0;
	naturalweapons=NULL;
	name="";
	symb='?';
	//y=0;
	//x=0;
	isplayer=0;
	viewdist=0;
	color=1;
      }
    }
  }

  file2.close();

  if (newspelllist != NULL) {list_delete(&newspelllist);}

  while(!file3.eof()) {

    file3.getline(line,128);
    char * tok;
    tok = strtok (line," ");
    if (tok != NULL) {
      if (strcmp(tok,"[name]") == 0) {
	tok = strtok (NULL,"\n");
	name = tok;
      }
      else if (strcmp(tok,"[mysteryname]") == 0) {
	tok = strtok (NULL,"\n");
	mysteryname = tok;
      }
      else if (strcmp(tok,"[symb]") == 0) {
	tok = strtok (NULL," ");
	symb = *tok;
      }
      else if (strcmp(tok,"[x,y]") == 0) {
	tok = strtok (NULL,",");
	x = atoi(tok);
	tok = strtok (NULL,",");
	y = atoi(tok);
      }
      else if (strcmp(tok,"[type]") == 0) {
	tok = strtok (NULL," ");
	itemtype = tok;
      }
      else if (strcmp(tok,"[mass]") == 0) {
	tok = strtok (NULL," ");
	mass = atoi(tok);
      }
      else if (strcmp(tok,"[size]") == 0) {
        tok = strtok (NULL," ");
        itemsize = atoi(tok);
      }
      else if (strcmp(tok,"[hplost]") == 0) {
	tok = strtok (NULL," ");
	hplost = atoi(tok);
      }
      else if (strcmp(tok,"[blockchance]") == 0) {
	tok = strtok (NULL," ");
	blockchance = atoi(tok);
      }
      else if (strcmp(tok,"[spell]") == 0) {
	tok = strtok (NULL,"\n");
	temp = tok;
	spellcastbyitem = objget->spellbyname(temp);
      }
      else if (strcmp(tok,"[uses]") == 0) {
	tok = strtok (NULL," ");
	uses = atoi(tok);
      }
      else if (strcmp(tok,"[hitbonus]") == 0) {
	tok = strtok (NULL," ");
	hitbonus = atoi(tok);
      }
      else if (strcmp(tok,"[dmgbonus]") == 0) {
	tok = strtok (NULL," ");
	dmgbonus = atoi(tok);
      }
      else if (strcmp(tok,"[fuel]") == 0) {
	tok = strtok (NULL," ");
	consumes = (i_fuel)(atoi(tok));
      }
      else if (strcmp(tok,"[color]") == 0) {
	tok = strtok (NULL," ");
	color = atoi(tok);
      }
      else if (strcmp(tok,"[subtype]") == 0) {
	tok = strtok (NULL," ");
	subcat = tok;
      }
      else if (strcmp(tok,"[effects]") == 0) {
	for (k=0;k<NUMEFFECTS;k++) {
	  tok = strtok (NULL," ");
	  effects[k] = atoi(tok);
	}
      }
      else if (strcmp(tok,"[dmgroll]") == 0) {
	for (k=0;k<DMGTYPES;k++) {
	  tok = strtok (NULL," ");
	  setdmg.rolls[k] = atoi(tok);
	}
      }
      else if (strcmp(tok,"[dmgvar]") == 0) {
	for (k=0;k<DMGTYPES;k++) {
	  tok = strtok (NULL," ");
	  setdmg.var[k] = atoi(tok);
	}
      }
      else if (strcmp(tok,"[end]") == 0) {
	Item *item = new Item(this, y, x, symb, name);

	item->setspell(spellcastbyitem);
	spellcastbyitem=NULL;
	item->settype(itemtype);
	item->setsubcat(subcat);
	item->setmass(mass);
	item->uses=uses;
	item->hplost=hplost;
	item->hitbonus=(char)hitbonus;
	item->dmgbonus=(char)dmgbonus;
	item->size = itemsize;
	item->consumes = consumes;
	consumes=none;
	hitbonus=0;
	itemsize=100;
	dmgbonus=0;
	hplost=0;
	uses=0;
	item->setdamage(setdmg);
	item->color = color;
	item->setblockchance(((char)blockchance));

	effpointer = item->geteffects();
	for (k=0;k<NUMEFFECTS;k++) {
	  *effpointer++ = effects[k];
	}

	if (mysteryname.compare("none") != 0) {
	  item->mysteryname = mysteryname;
	}

	name="";
	symb='?';
	y=0;
	x=0;
	viewdist=0;
	blockchance=0;
	mass =1;
	itemtype="none";
	subcat = "none";
	for (k=0;k<NUMEFFECTS;k++) {
	  effects[k]=0;
	}
	for (k=0;k<DMGTYPES;k++){
	  setdmg.rolls[k] = 0;
	  setdmg.var[k] = 0;
	}
      }
    }

  }

  return 1;

}

// Unsave (delete) function

void Map::unsave()
{
  string mapto, creaturefile, itemfile;
  mapto = "save/map.m";
  creaturefile = "save/creatures.m";
  itemfile = "save/items.m";

  remove(mapto.c_str());
  remove(creaturefile.c_str());
  remove(itemfile.c_str());

  char tch='a';
  while (maxdlvl > 0) {
    remove((mapto+tch).c_str());
    remove((creaturefile+tch).c_str());
    remove((itemfile+tch).c_str());
    tch++;
    maxdlvl--;
  }

  remove("save/itemknowledge.txt");
}

// Save function

int Map::savemap()
{
  return this->savemap(0);
}

int Map::savemap(int lvl)
{
  string mapto, creaturefile, itemfile;
  mapto = "save/map.m";
  creaturefile = "save/creatures.m";
  itemfile = "save/items.m";

  if (lvl > 0) {
    char tch='a';
    tch += (char)(lvl-1);
    mapto += tch;
    creaturefile += tch;
    itemfile += tch;
  }

  ofstream file1 (mapto.c_str());
  ofstream file2 (creaturefile.c_str());
  ofstream file3 (itemfile.c_str());
  ofstream file4 ("save/itemknowledge.txt");
  int monid=0;
  int itemid=0;
  char * effects;

  if ( !file1.is_open() || !file2.is_open() || !file3.is_open() || !file4.is_open()) {return 0;}

  file1 << seed << "\n";

  file1 << ysize << "\n";

  file1 << xsize << "\n";

  file1 << (int)dlvl << "\n";

  file1 << (int)maxdlvl << "\n";

  int i,j,k;
  for (j=0;j<ysize;j++) {

    for (i=0;i<xsize;i++) {

      // Use @'s as delimiters because they won't get
      // used as map features

      //      file1 << "@";

      file1 << map[j][i].symbol << map[j][i].visible;
      
      file1 << map[j][i].passable << map[j][i].seen;

      file1 << map[j][i].bloody << map[j][i].seethrough;

      file1 << map[j][i].opacity;

      file1 << (int)(map[j][i].color);
      
      file1 << map[j][i].seensymbol << map[j][i].shootthrough;
      
      file1 << "@";

      if (map[j][i].creature != NULL) {

	Creature * creature=NULL;
	creature = map[j][i].creature;
	string name=creature->name;

	file2 << "[name] ";

	file2 << name;

	file2 << "\n";
	
	file2 << "[symb] " << (map[j][i].creature)->getsymb() << "\n";

	file2 << "[x,y] " << (map[j][i].creature)->x << "," << (map[j][i].creature)->y << "\n";

	file2 << "[isplayer] " << (map[j][i].creature)->isplayer << "\n";

	if ((map[j][i].creature)->onfire) {
	  file2 << "[onfire]\n";
	}

	if ((map[j][i].creature)->frozen) {
	  file2 << "[frozen]\n";
	}

	if ((map[j][i].creature)->special) {
	  file2 << "[special]\n";
	}

	if ((map[j][i].creature)->isplayer == 0) {
	  file2 << "[target] " << ((Monster*)(map[j][i].creature))->targety << " ";
	  file2 << ((Monster*)(map[j][i].creature))->targetx << "\n";
	  file2 << "[active] " << ((Monster*)(map[j][i].creature))->active << "\n";
	}
	
	file2 << "[viewdist] " << (map[j][i].creature)->viewdist << "\n";

	file2 << "[strength] " << creature->strength << "\n";
	
	file2 << "[dexterity] " << creature->dexterity << "\n";
	
	file2 << "[agility] " << creature->agility << "\n";
	
	file2 << "[toughness] " << creature->toughness << "\n";
	
	file2 << "[size] " << creature->size << "\n";
	
	file2 << "[level] " << creature->level << "\n";

	file2 << "[experience] " << creature->experience << "\n";

	file2 << "[time] " << (int)(creature->time) << "\n";
	
	file2 << "[maxhp] " << creature->maxhp << "\n";
	
	file2 << "[hp] " << creature->hp << "\n";

	file2 << "[poison] " << creature->poisoned << "\n";

        file2 << "[bleeding] " << creature->bleeding << "\n";

	file2 << "[healrate] " << creature->healrate << "\n";

	file2 << "[healcount] " << creature->healcount << "\n";

	file2 << "[maxmp] " << creature->maxmp << "\n";
	
	file2 << "[mp] " << creature->mp << "\n";

	file2 << "[skills]";

	for (k=0;k<NUMSKILLS;k++) {
	  file2 << " " << (int)(creature->skills[k]);
	}

	file2 << "\n";

	file2 << "[rechargerate] " << creature->rechargerate << "\n";

	file2 << "[rechargecount] " << creature->rechargecount << "\n";

	file2 << "[color] " << (int)(creature->color) << "\n";

	if (*(creature->spells()) != NULL) {
	  
	  llist * spells = *(creature->spells());
	  while (spells != NULL) {
	    file2 << "[spell] " << ((Spell *)(list_item(&spells)))->name << "\n";
	    spells=*(&spells->next);
	  }
	  
	}

	if (creature->natwepnum > 0) {
	  for (k=0;k<(creature->natwepnum);k++) {
	    switch ((creature->naturalweapons)[k]) {
	    case bite: file2 << "[bite]\n"; break;
	    case claw: file2 << "[claw]\n"; break;
	    case slam: file2 << "[slam]\n"; break;
	    case horns: file2 << "[horns]\n"; break;
	    }
	  }
	}
	
	file2 << "[end]\n";

	llist * poss = *(creature->inventory());
	if (poss != NULL) {
	  Item *item = (Item *) list_item(&poss);
	  while (poss != NULL) {

	    file2 << "[item]\n";
	    
	    file2 << "[name] " << item->name << "\n";

	    file2 << "[mysteryname] " << item->mysteryname << "\n";
	    
	    file2 << "[symb] " << item->getsymb() << "\n";

	    file2 << creature->savepos(item);

	    file2 << "[type] " << item->gettype() << "\n";

	    file2 << "[mass] " << item->getmass() << "\n";

            file2 << "[size] " << item->size << "\n";

	    file2 << "[blockchance] " << (int)(item->getblockchance()) << "\n";

	    file2 << "[subtype] " << item->getsubcat() << "\n";

	    file2 << "[dmgroll]";
	    
	    for (k=0;k<DMGTYPES;k++) {
	      file2 << " " << (int)(item->getdamage(-1)).rolls[k];
	    }
	    file2 << "\n";

	    file2 << "[effects]";

	    effects = item->geteffects();

	    for (k=0;k<NUMEFFECTS;k++) {
	      file2 << " " << (int)*effects++;
	    }

	    file2 << "\n";

	    file2 << "[spell] " << item->getspellname() << "\n";

	    file2 << "[uses] " << item->uses << "\n";

	    file2 << "[hitbonus] " << (int)(item->hitbonus) << "\n";

	    file2 << "[dmgbonus] " << (int)(item->dmgbonus) << "\n";

	    file2 << "[hplost] " << item->hplost << "\n";

	    file2 << "[fuel] " << (int)(item->consumes) << "\n";

	    file2 << "[dmgvar]";
	    for (k=0;k<DMGTYPES;k++) {
	      file2 << " " << (int)(item->getdamage(-1)).var[k];
	    }
	    file2 << "\n";

	    file2 << "[color] " << (int)(item->color) << "\n";
	    
	    file2 << "[enditem]\n";

	    poss = *(&poss->next);
	    item = (Item *) list_item(&poss);
	    
	  }
	  item=NULL;
	}

      }

      if (map[j][i].ilist != NULL) {
	llist * ilist;
	Item * item;
	ilist = map[j][i].ilist;
	item = (Item *) list_item(&ilist);
	while (item != NULL) {

	  file3 << "[name] " << item->name << "\n";

	  file3 << "[mysteryname] " << item->mysteryname << "\n";

	  file3 << "[symb] " << item->getsymb() << "\n";

	  file3 << "[x,y] " << item->x << "," << item->y << "\n";

	  file3 << "[type] " << item->gettype() << "\n";

	  file3 << "[mass] " << item->getmass() << "\n";

          file3 << "[size] " << item->size << "\n";

	  file3 << "[blockchance] " << (int)(item->getblockchance()) << "\n";

	  file3 << "[subtype] " << item->getsubcat() << "\n";

	  file3 << "[effects]";

	  effects = item->geteffects();
	  
	  for (k=0;k<NUMEFFECTS;k++) {
	    file3 << " " << (int)*effects++;
	  }
	  
	  file3 << "\n";

	  file3 << "[spell] " << item->getspellname() << "\n";

	  file3 << "[uses] " << item->uses << "\n";

	  file3 << "[fuel] " << (int)(item->consumes) << "\n";

	  file3 << "[hitbonus] " << (int)(item->hitbonus) << "\n";

	  file3 << "[dmgbonus] " << (int)(item->dmgbonus) << "\n";

	  file3 << "[hplost] " << item->hplost << "\n";

	  file3 << "[dmgroll]";
	  for (k=0;k<DMGTYPES;k++) {
	    file3 << " " << (int)(item->getdamage(-1)).rolls[k];
	  }
	  file3 << "\n";

	  file3 << "[dmgvar]";
	  for (k=0;k<DMGTYPES;k++) {
	    file3 << " " << (int)(item->getdamage(-1)).var[k];
	  }
	  file3 << "\n";

	  file3 << "[color] " << (int)(item->color) << "\n";

	  file3 << "[end]\n";

	  ilist = *(&ilist->next);
	  item = (Item *) list_item(&ilist);
	}
	item=NULL;
      }

    }

    //    file1 << "\n";
    
  }

  file4 << (knowledge).c_str() << "\n";
  for (k=0;k<objget->numpots;k++) {
    file4 << objget->potmystery[k] + "\n";
  }
  for (k=0;k<objget->numspells;k++) {
    file4 << objget->wandmystery[k] + "\n";
  }
  for (k=0;k<objget->numspells;k++) {
    file4 << objget->scrollmystery[k] + "\n";
  }

  file1.close();
  file2.close();
  file3.close();
  file4.close();

  return 1;

}


/*************************************
   AOE FUNCTIONS.

Must be in format

int aoefunc(Map *map, int y, int x, int action)

*map is pointer to map (obviously)
y and x and coordinates.

action is what to do.
0

 ************************************/

int see(Map *map, int y, int x, int action)
{
  switch (action)
    {
    case 1:
      return map->seethrough(y,x);
    default:
      map->see(y,x);
      int i, seethrough, prev;
      seethrough = map->seethrough(y,x);
      int dx[5]={0,1,0,-1,0};
      int dy[5]={1,0,-1,0,1};
      prev=0;
      if (seethrough != 0) {
	for (i=0;i<5;i++) {
	  seethrough = map->seethrough((y+dy[i]),(x+dx[i]));
	  if (seethrough == 0) {
	    map->see((y+dy[i]),(x+dx[i]));
	    if (prev==1) {
	      map->see((y+dy[i]+dy[i-1]),(x+dx[i]+dx[i-1]));
	    }
	    prev=1;
	  }
	  else {
	    prev=0;
	  }
	}
      }
    }
  return map->getopacity(y,x);
}

int hit(Map *map, int y, int x, int action)
{
  switch (action)
    {
    case 1:
      if (map->checkmonster(y,x) == NULL && map->canshoot(y,x) == 1) {return 1;}
      else if (map->canshoot(y,x) == 0) {return 3;}
      else{return 0;}
    default: {}
    }
  return 2;
}

int spellsee(Map *map, int y, int x, int action)
{
  switch (action)
    {
    case 1:
      if (map->seethrough(y,x) == 0) {
	return 3;
      }
      else {
	return 1;
      }
    default:
      if (map->seethrough(y,x) != 0) {
	map->seteffected(y,x,1);
      }
    }
  return map->getopacity(y,x);
}

int spellhit(Map *map, int y, int x, int action)
{
  switch (action)
    {
    case 1:
      if (map->checkmonster(y,x) == NULL && map->canshoot(y,x) == 1) {return 1;}
      else if (map->canshoot(y,x) == 0) {return 3;}
      else{return 0;}
    default: {if (map->canshoot(y,x) == 1) {map->seteffected(y,x,1);}}
    }
  return 2;
}

int bloodtrail(Map *map, int y, int x, int action)
{  
  switch (action)
    {
    case 1:
      if (map->cango(y,x) == 1) {return 1;}
      else{return 3;}
    default: {if (map->cango(y,x) == 1) {map->makebloody(y,x);}}
    }
  return 2;
}



int fuck(Map *map, int y, int x, int action)
{
  if (y > 10) {map->see(y,x);}
}
