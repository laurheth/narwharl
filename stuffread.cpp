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
#include <cstring>
#include <sstream>

using namespace std;

extern Stuffgetter * objget;

Stuffgetter::Stuffgetter()
{
  mondefs=NULL;
  weapondefs=NULL;
  launcherdefs=NULL;
  armordefs=NULL;
  potiondefs=NULL;
  spells=NULL;
  nummons = getlist(&mondefs,"monsters.txt");
  numweps = getlist(&weapondefs,"weapons.txt");
  numlaunch = getlist(&launcherdefs,"launchers.txt");
  numarmors = getlist(&armordefs,"armor.txt");
  numpots = getlist(&potiondefs,"potions.txt");
  numspells=0;

  spellget(&spells,"spells.txt");

  llist * temp;
  temp = spells;
  while (temp != NULL) {
    numspells++;
    temp=*(&temp->next);
  }
  //  numspells=3;
  temp=NULL;

  int i;
  int j;
  int okay;
  string maybename;
  scrollmystery = new string[numspells];
  if (numspells > 0) {
    for (i=0;i<numspells;i++) {
      okay=(int)(3*mtrand())+2;
      maybename = "scroll of ";
      for (j=0;j<okay;j++) {
	char newchar;
	newchar = (int)(26*mtrand()+1)+96;
	if (mtrand()>0.8) {newchar -= 32;}
	maybename.append(1,newchar);
      }
      scrollmystery[i]=maybename;
    }
  }

  wandmystery = new string[numspells];

  maybename=this->wandname();
  wandmystery[0]=maybename;
  if (numspells>1) {
    i=1;
    while (i<(numspells)) {
      okay=1;
      j=i;
      maybename = this->wandname();
      while (j>0) {
	if (maybename.compare(wandmystery[j-1]) == 0) {
	  okay=0;
	}
	j--;
      }
      if (okay==1) {
	wandmystery[i]=maybename;
	i++;
      }
    }
  }
  

  potmystery = new string[numpots];
  maybename=this->potionname();
  potmystery[0]=maybename;

  if (numpots>1) {
    i=1;
    while (i<(numpots)) {
      okay=1;
      j=i;
      maybename = this->potionname();
      while (j>0) {
	if (maybename.compare(potmystery[j-1]) == 0) {
	  okay=0;
	}
	j--;
      }
      if (okay==1) {
	potmystery[i]=maybename;
	i++;
      }
    }
  }
}

Stuffgetter::~Stuffgetter()
{
}

string Stuffgetter::wandname()
{
  int i;
  string wandname;
  i = (int)(21*mtrand());
  switch(i) {
  case 0: wandname="lead"; break;
  case 1: wandname="glass"; break;
  case 2: wandname="iron"; break;
  case 3: wandname="plastic"; break;
  case 4: wandname="rubber"; break;
  case 5: wandname="feather"; break;
  case 6: wandname="wooden"; break;
  case 7: wandname="steel"; break;
  case 8: wandname="golden"; break;
  case 9: wandname="bismuth"; break;
  case 10: wandname="silver"; break;
  case 11: wandname="bronze"; break;
  case 12: wandname="copper"; break;
  case 13: wandname="bone"; break;
  case 14: wandname="rock"; break;
  case 15: wandname="ice"; break;
  case 16: wandname="plexiglass"; break;
  case 17: wandname="bone china"; break;
  case 18: wandname="emerald"; break;
  case 19: wandname="ruby"; break;
  case 20: wandname="sapphire"; break;
  case 21: wandname="amethyst"; break;
  }

  i = (int)(21*mtrand());
  switch(i) {
  case 0: wandname="runed " + wandname; break;
  case 1: wandname="jewel encrusted " + wandname; break;
  case 2: wandname="coiled " + wandname; break;
  case 3: wandname="diamond tipped " + wandname; break;
  case 4: wandname="glowing " + wandname; break;
  case 5: wandname="vibrating " + wandname; break;
  case 6: wandname="cold " + wandname; break;
  case 7: wandname="warm " + wandname; break;
  case 8: wandname="pointy " + wandname; break;
  case 9: wandname="rusty " + wandname; break;
  case 10: wandname="dull " + wandname; break;
  case 11: wandname="sticky " + wandname; break;
  case 12: wandname="painted " + wandname; break;
  case 13: wandname="short " + wandname; break;
  case 14: wandname="long " + wandname; break;
  case 15: wandname="curved " + wandname; break;
  case 16: wandname="bent " + wandname; break;
  case 17: wandname="spine covered " + wandname; break;
  case 18: wandname="sparkling " + wandname; break;
  case 19: wandname="flexible " + wandname; break;
  case 20: wandname="fluorescent " + wandname; break;
  }
  
  wandname += " wand";
  return wandname;
}

string Stuffgetter::potionname()
{
  int i;
  string potionname;
  i=(int)(20*mtrand());
  switch(i){
  case 0: potionname="thick"; break;
  case 1: potionname="runny"; break;
  case 2: potionname="bubbling"; break;
  case 3: potionname="warm"; break;
  case 4: potionname="cold"; break;
  case 5: potionname="fizzy"; break;
  case 6: potionname="foamy"; break;
  case 7: potionname="clumpy"; break;
  case 8: potionname="glowing"; break;
  case 9: potionname="pulpy"; break;
  case 10: potionname="carbonated"; break;
  case 11: potionname="opaque"; break;
  case 12: potionname="translucent"; break;
  case 13: potionname="volatile"; break;
  case 14: potionname="aromatic"; break;
  case 15: potionname="pungent"; break;
  case 16: potionname="metallic"; break;
  case 17: potionname="emulsive"; break;
  case 18: potionname="creamy"; break;
  case 19: potionname="vibrating"; break;
  }

  i=(int)(7*mtrand());
  switch (i) {
  case 0: potionname=potionname + " blue"; break;
  case 1: potionname=potionname + " brown"; break;
  case 2: potionname=potionname + " red"; break;
  case 3: potionname=potionname + " green"; break;
  case 4: potionname=potionname + " white"; break;
  case 5: potionname=potionname + " purple"; break;
  case 6: potionname=potionname + " cyan"; break;
  }

  potionname = potionname + " potion";
  return potionname;

}

Creature *Stuffgetter::addcreature(int level)
{
  Creature * creature;
  llist * tempmonlist;
  int i;
  int acceptable=0;
  int f;
  mondef * mon;
  while (acceptable == 0) {
    tempmonlist = mondefs;
    f = (int)(nummons * mtrand());
    if (f>0) {
      for (i=0;i<f;i++) {
	tempmonlist = *(&tempmonlist->next);
      }
    }
    mon = ((mondef*)list_item(&tempmonlist));
    if ((*mon).level <= level) {acceptable=1;}
  }
  creature = getcreature((*mon).ware,NULL,0,0);
  return creature;
}

Item *Stuffgetter::addscroll(int level)
{
  int i;
  Item *newscroll;
  Spell * spelltobe;

  do {
    i = (int)(numspells*mtrand());
    spelltobe = this->spellbyid(i);
  } while (spelltobe->level > level);


  newscroll = new Item(NULL, 0, 0, '?', "Scroll");
  newscroll->settype("scroll");
  newscroll->uses=1;
  newscroll->setspell(spelltobe);
  string newname = "scroll of " + spelltobe->name;
  newscroll->name = newname;
  newscroll->mysteryname = scrollmystery[i];
  return newscroll;
}

Item *Stuffgetter::addwand(int level)
{
  int i;
  int wandlevel;
  Item *newwand;
  Spell * spelltobe;
  wandlevel=100;

  do {
    i = (int)(numspells*mtrand());
    spelltobe = this->spellbyid(i);
    wandlevel=spelltobe->level;
  } while (wandlevel > level);


  newwand = new Item(NULL, 0, 0, '/', "Wand");
  newwand->settype("wand");
  newwand->uses=7+(int)(4*(mtrand()-mtrand()));
  newwand->setspell(spelltobe);
  string newname = "wand of " + spelltobe->name;
  newwand->name = newname;
  string mysteryname = wandmystery[i];
  newwand->mysteryname = mysteryname;

  if (mysteryname.find("rusty")!=string::npos) {newwand->color=(char)3;}
  else if (mysteryname.find("paint")!=string::npos) {newwand->color=(char)(2+5*mtrand());}
  else {
    if (mysteryname.find("plastic")!=string::npos) {newwand->color=(char)6;}
    else if (mysteryname.find("feather")!=string::npos) {newwand->color=(char)2;}
    else if (mysteryname.find("wooden")!=string::npos) {newwand->color=(char)2;}
    else if (mysteryname.find("golden")!=string::npos) {newwand->color=(char)(-2);}
    else if (mysteryname.find("bronze")!=string::npos) {newwand->color=(char)2;}
    else if (mysteryname.find("copper")!=string::npos) {newwand->color=(char)2;}
    else if (mysteryname.find("ice")!=string::npos) {newwand->color=(char)7;}
    else if (mysteryname.find("emerald")!=string::npos) {newwand->color=(char)4;}
    else if (mysteryname.find("ruby")!=string::npos) {newwand->color=(char)3;}
    else if (mysteryname.find("sapphire")!=string::npos) {newwand->color=(char)5;}
    else if (mysteryname.find("amethyst")!=string::npos) {newwand->color=(char)6;}
  }

  if ( (mysteryname.find("fluorescent")!=string::npos) | ((mysteryname.find("glowing")!=string::npos) | (mysteryname.find("sparkling")!=string::npos))) {newwand->color=-(newwand->color);}

  return newwand;
}

Item *Stuffgetter::addmissile(i_fuel ammotype)
{
  Item * missile;
  string misname;
  damage mdamage;

  missile = new Item(NULL, 0, 0, '(', "asdf");
  missile->settype("missile");
  mdamage = missile->getdamage(-1);

  switch (ammotype) {
    
  case arrow: {misname="arrow"; mdamage.rolls[1]=1; mdamage.var[1]=3; break;}
  default: {misname="arrow"; mdamage.rolls[1]=1; mdamage.var[1]=3;}
    
  }

  missile->consumes = ammotype;
  missile->setdamage(mdamage);
  missile->name = misname;
  missile->mysteryname = misname;

  return missile;

}

Item *Stuffgetter::addmissile(int level)
{
  i_fuel ammotype;

  ammotype = (i_fuel)((int)(AMMOTYPES * mtrand()) + 1);

  return this->addmissile(ammotype);

}

Item *Stuffgetter::addlauncher(int level)
{
  Item * weapon;
  llist * tempitemlist;
  int i;
  int acceptable=0;
  int f;
  mondef * item;
  while (acceptable == 0) {
    tempitemlist = launcherdefs;
    f = (int)(numlaunch * mtrand());
    if (f>0) {
      for (i=0;i<f;i++) {
	tempitemlist = *(&tempitemlist->next);
      }
    }
    item = ((mondef*)list_item(&tempitemlist));
    if ((*item).level <= level) {acceptable=1;}
  }
  weapon = getitem((*item).ware,NULL,0,0,"launchers.txt");
  return weapon;
}

Item *Stuffgetter::addweapon(int level)
{
  Item * weapon;
  llist * tempitemlist;
  int i;
  int acceptable=0;
  int f;
  mondef * item;
  while (acceptable == 0) {
    tempitemlist = weapondefs;
    f = (int)(numweps * mtrand());
    if (f>0) {
      for (i=0;i<f;i++) {
	tempitemlist = *(&tempitemlist->next);
      }
    }
    item = ((mondef*)list_item(&tempitemlist));
    if ((*item).level <= level) {acceptable=1;}
  }
  weapon = getitem((*item).ware,NULL,0,0,"weapons.txt");
  return weapon;
}

Item *Stuffgetter::addarmor(int level)
{
  Item * armor;
  llist * tempitemlist;
  int i;
  int acceptable=0;
  int f;
  mondef * item;
  while (acceptable == 0) {
    tempitemlist = armordefs;
    f = (int)(numarmors * mtrand());
    if (f>0) {
      for (i=0;i<f;i++) {
	tempitemlist = *(&tempitemlist->next);
      }
    }
    item = ((mondef*)list_item(&tempitemlist));
    if ((*item).level <= level) {acceptable=1;}
  }
  armor = getitem((*item).ware,NULL,0,0,"armor.txt");
  return armor;
}

Item *Stuffgetter::addpotion(int level)
{
  Item * potion;
  llist * tempitemlist;
  string mysteryname;
  int i;
  int acceptable=0;
  int f;
  mondef * item;
  while (acceptable == 0) {
    tempitemlist = potiondefs;
    f = (int)(numpots * mtrand());
    if (f>0) {
      for (i=0;i<f;i++) {
	tempitemlist = *(&tempitemlist->next);
      }
    }
    item = ((mondef*)list_item(&tempitemlist));
    if ((*item).level <= level) {acceptable=1;}
  }
  potion = getitem((*item).ware,NULL,0,0,"potions.txt");

  mysteryname=potmystery[f];

  potion->setmysteryname(mysteryname);
  
  if (mysteryname.find("white")!=string::npos) {potion->color=(char)1;}
  else if (mysteryname.find("brown")!=string::npos) {potion->color=(char)2;}
  else if (mysteryname.find("red")!=string::npos) {potion->color=(char)3;}
  else if (mysteryname.find("green")!=string::npos) {potion->color=(char)4;}
  else if (mysteryname.find("blue")!=string::npos) {potion->color=(char)5;}
  else if (mysteryname.find("purple")!=string::npos) {potion->color=(char)6;}
  else if (mysteryname.find("cyan")!=string::npos) {potion->color=(char)7;}

  if (mysteryname.find("glowing")!=string::npos) {potion->color=-(potion->color);}
  
  return potion;
}

Spell *Stuffgetter::spellbyid(int idnum)
{
  Spell * spell;
  llist * tempspelllist;
  tempspelllist=spells;
  int i;
  if (idnum > 0) {
    for (i=0;i<idnum;i++) {
      tempspelllist=*(&tempspelllist->next);
    }
  }
  return (Spell*)list_item(&tempspelllist);
}

Spell *Stuffgetter::spellbyname(string spellname)
{
  Spell * spell;
  string nametocompare;
  llist * tempspelllist;
  tempspelllist=spells;
  while (tempspelllist != NULL) {
    spell=(Spell*)list_item(&tempspelllist);
    nametocompare=spell->name;
    if (nametocompare.compare(spellname)==0) {
      return spell;
    }
    tempspelllist=*(&tempspelllist->next);
  }
  return NULL;
}

llist **Stuffgetter::allspells()
{
  return &spells;
}

int spellget (llist ** stuffs, string deffile)
{
  ifstream file (("defs/" + deffile).c_str());
  if (!file.is_open()) {return 0;}

  char line[128];

  int count=0;

  while(!file.eof()) {
    file.getline(line,128);
    char * tok;
    tok = strtok(line, " ");
    if (tok != NULL) {
      if (strcmp(tok,"[begin]") == 0) {
	Spell * newspell;
	newspell = new Spell();
	while(!file.eof()) {
	  file.getline(line,128);
	  tok = strtok(line, " ");
	  if (strcmp(tok,"[name]") == 0) {
	    tok = strtok(NULL,"\n");
	    newspell->name = tok;
	  }
	  else if (strcmp(tok,"[effect]") == 0) {
	    tok = strtok(NULL," ");
	    effecttype spelleffect;
	    switch (atoi(tok)) {
	      case 1: spelleffect=firedmg; break;
	      case 2: spelleffect=icedmg; break;
	      case 3: spelleffect=identifyitem; break;
	      default: spelleffect=noeffect;
	    }
	    newspell->effect = spelleffect;
	  }
	  else if (strcmp(tok,"[range]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->range = atoi(tok);
	  }
	  else if (strcmp(tok,"[skiprange]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->skiprange = atoi(tok);
	  }
	  else if (strcmp(tok,"[spread]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->spread = atoi(tok);
	  }
	  else if (strcmp(tok,"[blastradius]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->blastradius = atoi(tok);
	  }
	  else if (strcmp(tok,"[stopattarget]") == 0) {
            tok = strtok(NULL," ");
            newspell->stopattarget = atoi(tok);
          }
	  else if (strcmp(tok,"[level]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->level = atoi(tok);
	  }
	  else if (strcmp(tok,"[color]") == 0) {
	    tok = strtok(NULL," ");
	    newspell->color = (char)atoi(tok);
	  }
	  else if (strcmp(tok,"[args]") == 0) {
	    tok = strtok(NULL," ");
	    int argpos;
	    argpos=-1;
	    while (tok != NULL) {
	      if (argpos == -1) {
		argpos = atoi(tok);
	      }
	      else {
		newspell->args[argpos] = atoi(tok);
		argpos=-1;
	      }
	      tok = strtok(NULL," ");
            }
	  }
	  else if (strcmp(tok,"[end]") == 0) {
	    list_append(stuffs,(void *)newspell);
	    newspell=NULL;
	    break;
	  }
	}
      }
    }
  }
  return 1;
}

int getlist (llist ** stuffs, string deffile)
{

  ifstream file (("defs/" + deffile).c_str());
  if (!file.is_open()) {return 0;}
  
  char line[128];

  int count=0;

  streampos ware=-1;
  int level=0;

  while(!file.eof()) {

    file.getline(line,128);
    char * tok;
    tok = strtok(line, " ");
    if (tok != NULL) {
      if (strcmp(tok,"[begin]") == 0) {
	ware = file.tellg();
      }
      else if (strcmp(tok,"[level]") == 0) {
	tok = strtok (NULL, " ");
	level = atoi(tok);
      }
      else if (strcmp(tok,"[leveladjust]") == 0) {
	tok = strtok (NULL, " ");
	level += atoi(tok);
      }
      else if (strcmp(tok,"[end]") == 0) {
	if (ware != (streampos)(-1) && level != 0) {
	  // Create the monster def.
	  mondef * nextmon = new mondef;
	  (*nextmon).ware = ware;
	  (*nextmon).level = level;

	  list_add(stuffs,nextmon);
	  
	  // Set values back to ones representing nothing.
	  ware=-1;
	  level=0;
	  count++;
	}
      }
    }
  }

  return count;

}

Creature * getcreature(streampos ware, Map *map, int y, int x)
{
  ifstream file ("defs/monsters.txt");
  if (!file.is_open()) {return 0;}

  Creature * creature=NULL;

  char line[128];
  file.seekg(ware);
  string name;
  string spellknown;
  char symb;

  // -- For deciding what skills to hand out.
  int melee=0; // Is a melee fighter
  int ranged=0; // Is a ranged fighter
  int dodger=0; // Is a dodger
  int blocker=0; // Is a blocker
  int caster=0; // Is a caster
  int disarm=0; // Is a disarmer
  // --

  int isarmed=0;
  int color=1;
  int strength=10;
  int dexterity=10;
  int agility=10;
  int toughness=10;
  natwep * naturalweapons;
  naturalweapons=NULL;
  int natwepnums=0;
  int size=100;
  int level=1;
  int leveladjust=0;
  int healrate=10;
  int rechargerate=10;
  int maxmp=0;
  llist * newspelllist=NULL;
  while (!file.eof()) {
    file.getline(line,128);
    char * tok;
    tok = strtok(line," ");
    if (tok != NULL) {
      if (strcmp(tok,"[name]") == 0) {
	tok = strtok(NULL,"\n");
	name = tok;
      }
      else if (strcmp(tok,"[symb]") == 0) {
	tok = strtok(NULL," ");
	symb = *tok;
      }
      else if (strcmp(tok,"[color]") == 0) {
	tok = strtok(NULL," ");
	color = atoi(tok);
      }
      else if (strcmp(tok,"[armed]") == 0) {
	isarmed = 1;
      }
      else if (strcmp(tok,"[melee]") == 0) {
	melee = 1;
      }
      else if (strcmp(tok,"[ranged]") == 0) {
	ranged = 1;
      }
      else if (strcmp(tok,"[dodger]") == 0) {
	dodger = 1;
      }
      else if (strcmp(tok,"[blocker]") == 0) {
	blocker = 1;
      }
      else if (strcmp(tok,"[caster]") == 0) {
	caster = 1;
      }
      else if (strcmp(tok,"[disarm]") == 0) {
	disarm = 1;
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


      else if (strcmp(tok,"[strength]") == 0) {
	tok = strtok(NULL," ");
	strength = atoi(tok);
      }
      else if (strcmp(tok,"[dexterity]") == 0) {
	tok = strtok(NULL," ");
	dexterity = atoi(tok);
      }
      else if (strcmp(tok,"[agility]") == 0) {
	tok = strtok(NULL," ");
	agility = atoi(tok);
      }
      else if (strcmp(tok,"[toughness]") == 0) {
	tok = strtok(NULL," ");
	toughness = atoi(tok);
      }
      else if (strcmp(tok,"[size]") == 0) {
	tok = strtok(NULL," ");
	size = atoi(tok);
      }
      else if (strcmp(tok,"[level]") == 0) {
	tok = strtok(NULL," ");
	level = atoi(tok);
      }

      else if (strcmp(tok,"[leveladjust]") == 0) {
	tok = strtok(NULL," ");
	leveladjust = atoi(tok);
      }

      else if (strcmp(tok,"[rechargerate]") == 0) {
	tok = strtok(NULL," ");
	rechargerate = atoi(tok);
      }
      else if (strcmp(tok,"[maxmp]") == 0) {
	tok = strtok(NULL," ");
	maxmp = atoi(tok);
      }
      else if (strcmp(tok,"[healrate]") == 0) {
	tok = strtok(NULL," ");
	healrate = atoi(tok);
      }

      else if (strcmp(tok, "[spell]") == 0) {
	tok = strtok(NULL,"\n");
	spellknown=tok;
	list_append(&newspelllist, (void*)(objget->spellbyname(spellknown)));
      }

      else if (strcmp(tok,"[end]") == 0) {
	creature = new Monster(map, y, x, symb, name);
	creature->strength=strength;
	*(creature->spells()) = newspelllist;
	creature->color=color;
	creature->natwepnum=natwepnums;
	creature->naturalweapons=naturalweapons;
	creature->dexterity=dexterity;
	creature->agility=agility;
	creature->toughness=toughness;
	creature->size=size;
	creature->level=level;
	creature->leveladjust=leveladjust;
	creature->maxhp = ((level * toughness) * size) / 100;
	creature->hp = creature->maxhp;
	creature->maxmp = maxmp;
	creature->mp = maxmp;
	creature->healrate = healrate;
	creature->rechargerate = rechargerate;
	creature->time = (int)(4*mtrand());
	if (isarmed == 1) {
	  if (mtrand()<0.9) {
	    melee=1;
	    creature->lhand = objget->addweapon(level+1);
	  }
	  else {
	    ranged=1;
	    creature->lhand = objget->addlauncher(level+1);
	    i_fuel ammos = (creature->lhand)->consumes;
	    int numammos=(int)(4*mtrand())+3;
	    int i;
	    for (i=0;i<numammos;i++) {
	      list_add(creature->inventory(),(void *)(objget->addmissile(ammos)));
	    }
	  }
	  list_add(creature->inventory(),(void *)(creature->lhand));
	}
	if (melee == 1) {
	  creature->skills[0] = (char)(level);
	}
	if (ranged == 1) {
	  creature->skills[8] = (char)(level);
	}
	if (dodger == 1) {
	  creature->skills[1] = (char)(level);
	}
	if (blocker == 1) {
	  creature->skills[2] = (char)(level);
	}
	if (caster == 1) {
	  creature->skills[3] = (char)(level);
	}
	if (disarm == 1) {
	  creature->skills[9] = (char)(level);
	}
	break;
      }
    }
  }

  if (creature != NULL) {
    return creature;
  }
  else {
    list_delete(&newspelllist);
  }

}

Item * getitem(streampos ware, Map *map, int y, int x, string deffile)
{
  ifstream file (("defs/" + deffile).c_str());
  if (!file.is_open()) {return 0;}
  
  Item * item=NULL;
  
  char line[128];
  file.seekg(ware);
  string name;
  string mysteryname="none";
  int k;
  string subcat;
  string type;
  string temp;
  char symb='?';
  int color=1;
  int mass=1;
  int hitbonus=0;
  int dmgbonus=0;
  int level=1;
  int size=100;
  int blockchance=0;
  damage setdmg;
  char * effpointer;
  char effects[NUMEFFECTS];
  i_fuel consumes=none;
  for (k=0;k<NUMEFFECTS;k++) {
    effects[k]=0;
  }
  for (k=0;k<DMGTYPES;k++) {
    setdmg.rolls[k] = 0;
    setdmg.var[k] = 0;
  }
  while (!file.eof()) {
    file.getline(line,128);
    char * tok;
    tok = strtok(line," ");
    if (tok != NULL) {
      if (strcmp(tok,"[name]") == 0) {
	tok = strtok(NULL,"\n");
	name = tok;
      }
      if (strcmp(tok,"[mysteryname]") == 0) {
	tok = strtok(NULL,"\n");
	mysteryname = tok;
      }
      if (strcmp(tok,"[fuel]") == 0) {
	tok = strtok(NULL," ");
	temp = tok;
	if (temp.compare("arrow")==0) {consumes=arrow;}
      }
      else if (strcmp(tok,"[type]") == 0) {
	tok = strtok(NULL," ");
	type = tok;
      }
      else if (strcmp(tok,"[color]") == 0) {
	tok = strtok(NULL," ");
	color = atoi(tok);
      }
      else if (strcmp(tok,"[subtype]") == 0) {
	tok = strtok(NULL," ");
	subcat = tok;
      }
      else if (strcmp(tok,"[mass]") == 0) {
	tok = strtok(NULL," ");
	mass = atoi(tok);
      }
      else if (strcmp(tok,"[hitbonus]") == 0) {
	tok = strtok(NULL," ");
	hitbonus = atoi(tok);
      }
      else if (strcmp(tok,"[size]") == 0) {
        tok = strtok(NULL," ");
        size = atoi(tok);
      }
      else if (strcmp(tok,"[dmgbonus]") == 0) {
	tok = strtok(NULL," ");
	dmgbonus = atoi(tok);
      }
      else if (strcmp(tok,"[blockchance]") == 0) {
	tok = strtok(NULL," ");
	blockchance = atoi(tok);
      }
      else if (strcmp(tok,"[level]") == 0) {
	tok = strtok(NULL," ");
	level = atoi(tok);
      }      
      else if (strcmp(tok,"[effects]") == 0) {
	for (k=0;k<NUMEFFECTS;k++) {
	  tok = strtok(NULL," ");
	  effects[k] = (char)atoi(tok);
	}
      }
      else if (strcmp(tok,"[dmgroll]") == 0) {
	for (k=0;k<DMGTYPES;k++) {
	  tok = strtok(NULL," ");
	  setdmg.rolls[k] = atoi(tok);
	}
      }
      else if (strcmp(tok,"[dmgvar]") == 0) {
	for (k=0;k<DMGTYPES;k++) {
	  tok = strtok(NULL," ");
	  setdmg.var[k] = atoi(tok);
	}
      }
      else if (strcmp(tok,"[end]") == 0) {

	item = new Item(map, y, x, symb, name);

	if (mysteryname.compare("none") != 0) {
	  item->setmysteryname(mysteryname);
	}
	item->hitbonus=hitbonus;
	item->dmgbonus=dmgbonus;
	item->settype(type);
	item->setsubcat(subcat);
	item->setmass(mass);
	item->setdamage(setdmg);
	item->setblockchance((char)blockchance);
	item->color = color;
	item->size = size;
	item->consumes = consumes;
	effpointer = item->geteffects();
	for (k=0;k<NUMEFFECTS;k++) {
	  *effpointer++ = effects[k];
	}

	break;
      }
    }
  }
  
  return item;
  
}
