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
#include <string>

using namespace std;

extern string knowledge;

int globalid=0;

// Creature functions
// constructor
Creature::Creature(Map *map, int sy, int sx, char ssymb, string sname)
{
  attacker=-1;
  id=globalid++;
  color=WHITE; // Default color.
  name=sname; // Set name to given name.
  symb=ssymb; // Symbol.
  y = sy; // Position
  x = sx;
  viewdist = 9; // Default view distance
  if (x != 0 && y != 0) {
    map->go(this,y,x);
  } // Place the creature, unless x and y are both 0.

  int i;
  for (i=0;i<NUMSKILLS;i++) {
    skills[i]=0;
  }

  experience=0;
  leveladjust=0;
  time=0;
  sundered=0;
  disarmed=0;
  animation=0;
  onfire=0;
  special=0;
  frozen=0;

  // Default stats.
  strength = 10;
  dexterity = 10;
  agility = 10;
  toughness = 10;
  size = 100;
  level = 1;
  maxhp = 10;
  hp = 10;
  maxmp = 0;
  mp = 0;
  poisoned=0;
  isplayer = 0;
  bleeding=-1;
  healrate=10;
  healcount=600;
  rechargerate = 10;
  rechargecount = 200;
  spellist = NULL;
  possessions = NULL;
  larmor = NULL;
  lhand = NULL;
  rhand = NULL;
  lring = NULL;
  rring = NULL;
  naturalweapons = NULL;
  natwepnum=0;
  load=0;
}

// destructor
Creature::~Creature()
{
  Item * itemtorem;

  if (possessions != NULL) {
    while (possessions != NULL) {
      itemtorem = (Item *)list_item(&possessions);
      delete itemtorem;
      list_remove(&possessions);
    }
  }
  if (naturalweapons != NULL) {free(naturalweapons); naturalweapons=NULL; }
}

// Calculate encumberance.
void Creature::calcload()
{

  int weight=0;
  int cancarry;
  Item * item;
  cancarry=((strength+1) * (strength+1) * size)/200;

  llist * templist;
  templist = possessions;
  if (templist != NULL) {
    
    while (templist != NULL) {

      item = ((Item *)list_item(&templist));
      weight += item->getmass();
      templist=*(&templist->next);

    }

  }

  if (weight <= cancarry) {load=0; return;}

  load = weight - cancarry;

}

void Creature::increasestat(string stat)
{
  int * val;
  if (stat.compare("strength") == 0) {val = &strength;}
  else if (stat.compare("dexterity") == 0) {val = &dexterity;}
  else if (stat.compare("agility") == 0) {val = &agility;}
  else if (stat.compare("toughness") == 0) {val = &toughness;}
  else {return;}
  int prob = 30 - *val;
  if (prob > 6000 * mtrand()) {
    *val = *val + 1;
    if (stat.compare("toughness") == 0) {
      maxhp += level;
    }
  }
  this->calcload();
  return;

}

// Get skill
int Creature::getskill(string skillname)
{
  char val=-1;
  if (skillname.compare("melee") == 0) {val =  skills[0];}
  else if (skillname.compare("dodging") == 0) { val = skills[1];}
  else if (skillname.compare("blocking") == 0) { val = skills[2];}
  else if (skillname.compare("magic") == 0) { val = skills[3];}
  else if (skillname.compare("stealth") == 0) { val = skills[4];}
  else if (skillname.compare("parrying") == 0) { val = skills[5];}
  else if (skillname.compare("2wepfighting") == 0) { val = skills[6];}
  else if (skillname.compare("hardness") == 0) { val = skills[7];}
  else if (skillname.compare("ranged") == 0) { val = skills[8];}
  else if (skillname.compare("disarm") == 0) { val = skills[9];}
  return (int)val;
}

// Set skill
void Creature::setskill(string skillname, int newvalue)
{
  char val;
  val = (char)newvalue;

  if (skillname.compare("melee") == 0) {skills[0] = val;}
  else if (skillname.compare("dodging") == 0) { skills[1] = val;}
  else if (skillname.compare("blocking") == 0) { skills[2] = val;}
  else if (skillname.compare("magic") == 0) { skills[3] = val;}
  else if (skillname.compare("stealth") == 0) { skills[4] = val;}
  else if (skillname.compare("parrying") == 0) { skills[5] = val;}
  else if (skillname.compare("2wepfighting") == 0) { skills[6] = val;}
  else if (skillname.compare("hardness") == 0) { skills[7] = val;}
  else if (skillname.compare("ranged") == 0) { skills[8] = val;}
  else if (skillname.compare("disarm") == 0) { skills[9] = val;}

}

// Read scroll
void Creature::read(Item * item, Map * map, Screen * vscreen, int targy, int targx)
{
  time=(char)( (40 / agility) );
  item->cast(this, map, vscreen, 0, 0);
  delete item;
  list_remove(list_search(this->inventory(), (void *)item));
}

// Zap wand.
void Creature::zap(Item * item, Map * map, Screen * vscreen, int targy, int targx)
{
  time=(char)( (40 / agility) );
  item->cast(this, map, vscreen, 0, 0);
}

// Destroy possesion
string Creature::destroypos(Item * item)
{
  string message;
  string itemname;
  itemname = item->getname();
  if (isplayer == 0) {
    message = this->getname() + "'s " + itemname + " shatters into pieces!";
  }
  else {
    message = "Your " + itemname + " shatters into pieces!";
  }
  //  vscreen->message(message);
  if (lhand==item) { lhand=NULL; }
  if (rhand==item) { rhand=NULL; }
  list_remove(list_search(this->inventory(), (void *)item));
  delete item;
  this->calcload();
  return message;
}

void Creature::die(Map *map)
{
  map->pop--;
  if (mtrand()>0.9) {
    map->maxpop--;
  }
  string corpsename;
  corpsename = name;
  corpsename = corpsename + " corpse"; // Name of corpse.
  map->leave(this->y,this->x);
  llist * carried = *(this->inventory()); // Put inventory on the ground.
  if (possessions != NULL) {
    while (possessions != NULL) {
      list_add(map->getitems(y,x),list_item(&possessions));
      //carried = *(&carried->next);
      list_remove(&possessions);
    }
    //    list_delete(&possessions);
  }
  if (natwepnum > 0) {free(naturalweapons); naturalweapons=NULL; natwepnum=0;}
  Item *item = new Item(map, y, x, '%', corpsename); // Make item: Corpse!
  item->color = this->color+7; // Set its background to red.
  item->setmass(1+2*size/3);
  
  // Hand out experience.
  Creature * killer;
  if (attacker != id && attacker > -1) {
    // Search through the list of monsters

    int foundid;
    llist * temp;
    temp = *(map->mlist);
    while (temp != NULL && foundid != attacker) {
      killer = (Creature*)list_item(&temp);
      foundid=killer->id;
      if (foundid==attacker) {break;}
      else {foundid=-1;}
      temp = *(&temp->next);
    }
    if (foundid == attacker) {
      killer->experience += this->level;
    }

  }
}

// step in direction defined by dy and dx.
void Creature::step(Map *map, int dy, int dx, Screen *vscreen)
{
  Creature * creature;
  creature = map->checkmonster((y+dy),(x+dx)); // Check if monster is present at desired location.
  if ((map->cango((y+dy),(x+dx)) != 0) && ( creature == NULL) ) { // Go there, if possible.
    if (map->getfloorsymb(y+dy,x+dx) == '+') {
      map->opendoor(y+dy,x+dx);
      if (isplayer == 1) {
	vscreen->message("You open the door.");
      }
    }
    else {
      map->leave(y,x);
      x = x + dx;
      y = y + dy;
      map->go(this,y,x);
    }
  }
  else if (creature != NULL && creature != this) { // Melee attack!
    this->attack(creature, map, vscreen);
  }
}

// Get it's color.
char Creature::getcolor()
{

  // Alternate rapidly between yellow and red
  if (onfire==1) {
    animation--;
    if (animation % 3 == 0) {
      return -3;
    }
    else {
      return -2;
    }
  }

  // cyan!
  if (frozen==1) {
    return -7;
  }

  // Blink if you're special! TOTALLY NOT A DWARF FORTRESS RIP OFF
  if (special==1) {
    animation--;
    if (animation<0) {
      return color;
    }
    else {
      return -color;
    }
  }


  return color;
}

// Get it's symbol.
char Creature::getsymb()
{
  return symb;
}

// Get creatyre name.
string Creature::getname()
{
  char firstletter;
  string tobereturned;
  firstletter = name[0];
  if (firstletter > 96 && firstletter < 123) {
    tobereturned = "the " + name;
  }
  else {tobereturned = name;}
  return tobereturned;
}

// Definition of virtual function ai()
// Player has a bunch of stuff for commands.
// Monsters have ai scripts of some sort.
// Any other wierd things will have something else here.
int Creature::ai(Map *map, Screen *vscreen)
{
}

// Inventory related stuffs.

llist **Creature::inventory()
{
  return &possessions;
}

// Armor!

llist **Creature::armor()
{
  return &larmor;
}

// Spell list!

llist **Creature::spells()
{
  return &spellist;
}

// Wield!
// todo: Add a check for item type, as soon as item types are added.
int Creature::wield(Item * item)
{
  time=(char)( (40 / agility) );
  if (lhand == item || rhand == item) {return 2;} // Already wielding
  if (rring == item || lring == item) {return 3;} // Alreadying wearing
  if (list_search(this->armor(),(void *)item) != NULL) {return 3;} // Already wearing

  char twohand=2;
  int twohandsize=2*(this->size); // Twice as large or larger weapons are two handed. 
  if ((item->name).find("buckler")!=string::npos) {twohand=0;} // Special case. Bucklers can be used with two handed weapons.
  else {
    if (lhand == NULL && rhand == NULL) {twohand=0;} // If empty handed, no need to go through other checks.
    else {
      if (item->size >= twohandsize || (item->gettype()).compare("launcher") == 0) {twohand=2;} // If the weapon to be wielded is two handed, use the two hand case.
      else {
	// If the weapon is one handed and neither hand has a two handed weapon, then use the one handed case.
        if (lhand == NULL) {twohand--;}
	else if (lhand->size < twohandsize && (lhand->gettype()).compare("launcher") != 0) {twohand--;} // Left hand weapon is one handed
        if (rhand == NULL) {twohand--;}
	else if (rhand->size < twohandsize && (rhand->gettype()).compare("launcher") != 0) {twohand--;} // Right hand weapon is one handed
      }
    }
  }

  // Two handed case.
  if (twohand > 0) {

    Item ** whichhand=NULL;
    int bucklers=0;
    char freehands=0;
    if (lhand == NULL) {freehands++; whichhand=&lhand;}
    else if ((lhand->name).find("buckler")!=string::npos) {freehands++; bucklers++;} // Bucklers are a special case.

    if (rhand == NULL) {freehands++; whichhand=&rhand;}
    else if ((rhand->name).find("buckler")!=string::npos) {freehands++; bucklers++;} // Bucklers are a special case.

    if (freehands == 2 && whichhand != NULL) {
      *whichhand = item; // Put it into the appropriate free hand.
    }
    else {
      if (whichhand == &lhand) {
        return 4; // Need to free right hand!
      }
      else if (whichhand == &rhand) {
	return 5; // Need to free left hand!
      }
      else if (bucklers == 2) {
	return 4;
      }
      else {return 6;} // Need to free both hands.
    }

  } // One handed case.
  else {
    if (lhand == NULL) { // Left hand empty?
      lhand = item; // Put in left hand.
    }
    else if (rhand == NULL) { // Right hand empty?
      rhand = item; // Put in right hand.
    }
    else { return 0; } // No hands? Can't wield. Choose a hand to drop.
  }
  return 1; // success
}

// Unwield!
int Creature::unwield(Item * item)
{
  if (lhand == item) {
    lhand = NULL; // If it's in your left hand, clear left hand.
  }
  else if (rhand == item) {
    rhand = NULL; // If it's in your right hand, clear right hand.
  }
  else { return 0; } // Can't unwield. You aren't wielding it!
  return 1;
}

// Wear!
int Creature::wear(Item * item)
{
  time=(char)( (40 / agility) );
  if (lring == item || rring == item) {return 2;}
  if (lring == NULL) { // Left ring empty?
    lring = item; // Put on left ring.
  }
  else if (rring == NULL) { // Right ring empty?
    rring = item; // Put on right ring.
  }
  //  else if (neck == NULL) {
  //  neck = item;
  //}
  else { return 0; } // No hands? Can't wear.
  if (rhand == item) {rhand = NULL;}
  if (lhand == item) {lhand = NULL;}
  return 1;
}

// Unwear!
int Creature::unwear(Item * item)
{
  if (lring == item) {
    lring = NULL; // If it's on your left hand, clear left hand.
  }
  else if (rring == item) {
    rring = NULL; // If it's on your right hand, clear right hand.
  }
  else { return 0; } // Can't unwear. You aren't wearing it!
  return 1;
}

// Return where item is worn, if anywhere.
string Creature::isworn(Item * item)
{
  string location = " ";
  if (item == lhand) {location = "Left Hand";}
  else if (item == rhand) {location = "Right Hand";}
  else if (item == neck) {location = "Around Neck";}
  else if (item == lring) {location = "Left Ring";}
  else if (item == rring) {location = "Right Ring";}
  else if (list_search(&larmor, (void *)item) != NULL) {
    location = "Clothing/Armor"; }
  return location;
}

// Return name of armor of the requested subtype is already worn. Otherwise "none".
string Creature::wearingarmor(string armortype)
{
  string testedtype;
  Item * item;
  llist * templist=*(this->armor());
  if (templist != NULL) {
    while (templist != NULL) {
      item = (Item *)(list_item(&templist));
      testedtype = item->getsubcat();
      if (armortype.compare(testedtype) == 0) {return item->getname();}
      templist = *(&templist->next);
    }
  }
  return "none";
}

// Same as above but better formatted for savefiles.
string Creature::savepos(Item * item)
{
  string location;
  if (item == lhand) {location = "[lhand]\n";}
  else if (item == rhand) {location = "[rhand]\n";}
  else if (item == neck) {location = "[neck]\n";}
  else if (item == lring) {location = "[lring]\n";}
  else if (item == rring) {location = "[rring]\n";}
  else if (list_search(&larmor, (void *)item) != NULL) {
    location = "[worn]\n"; }
  else {location = "";}
  return location;
}

// Experience item effect (ohno.. or yes?)
string Creature::itemeffect(Item * potion)
{
  int change;
  char * effects;
  effects = potion->geteffects();
  string message = "-";
  int i;
  for (i=0;i<NUMEFFECTS;i++) {
    change = (int)effects[i];
    switch (i) { // Great big switch statement full of effects.
    case 0: {
      this->hp = this->hp + change;
      if (change>0) {
	if (isplayer == 1) {
	  message = "You feel better. ";
	}
	else {
	  message = this->getname() + "'s wounds close!";
	}
	this->poisoned = this->poisoned - 8*change;
	this->bleeding -= 4 * change;
	if (this->poisoned < 0) {this->poisoned = 0;}
        if (this->bleeding < 0) {this->bleeding = -1;}
      }
      else if (change<0) {
	if (isplayer == 1) {
	  message = "Old wounds open again. ";
	}
	else {
	  message = this->getname() + "'s skin breaks open!";
	}
      }
      break;
    }
    case 1: {
      this->size = (this->size) * (100 + change)/100;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "You become bigger. ";
	}
	else {
	  message = this->getname() + " grows larger!";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "The world seems larger. ";
	}
	else {
	  message = this->getname() + " shrinks!";
	}
      }
      break;
    }
    case 2: {
      this->strength = (this->strength) + change;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "You feel stronger. ";
	}
	else {
	  message = this->getname() + "'s muscles buldge!";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "You feel weak. ";
	}
	else {
	  message = this->getname() + " becomes thinner!";
	}
      }
      break;
    } 
    case 3: {
      this->dexterity = (this->dexterity) + change;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "Your vision improves. ";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "Everything is blurry. ";
	}
      }
      break;
    } 
    case 4: {
      this->agility = (this->agility) + change;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "You feel faster. ";
	}
	else {
	  message = this->getname() + " speeds up!";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "You become sluggish. ";
	}
	else {
	  message = this->getname() + " slows down!";
	}
      }
      break;
    } 
    case 5: {
      this->toughness = (this->toughness) + change;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "You become thicker skinned. ";
	}
	else {
	  message = this->getname() + " looks tougher!";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "You become more frail. ";
	}
	else {
	  message = this->getname() + " becomes more frail!";
	}
      }
      break;
    } 
    case 6: {
      this->poisoned = (this->poisoned) + 4*change;
      if (change > 0) {
	if (isplayer == 1) {
	  message = "You are poisoned! ";
	}
	else {
	  message = this->getname() + " looks sick!";
	}
      }
      else if (change < 0) {
	if (isplayer == 1) {
	  message = "You feel cleansed. ";
	}
	else {
	  message = this->getname() + " looks healthier!";
	}
      }
      break;
    }
    }
    
  }
  maxhp = ((level * toughness)*size)/100;
  if (hp>maxhp) hp=maxhp;
  return message;
 
}

// Quaff potion.
void Creature::quaff(Item *potion, Screen *vscreen)
{
  time=(char)( (40 / agility) );
  string message;
  message = "You drink the " + potion->getname() + ". ";

  message += this->itemeffect(potion);

  //    knowledge=knowledge+","+potion->name;
  potion->identify();
  
  list_remove(list_search(this->inventory(),(void *)potion));
  this->unwear(potion);
  this->unwield(potion);
  list_remove(list_search(this->armor(),(void *)potion));
  //delete potion;
  //potion=NULL;
  vscreen->message(message);
}

// Frozen D:
void Creature::freeze()
{
  if (frozen) {
    time++;
    if (mtrand()>0.99) {
      frozen=0;
    }
  }
}

// Burning (OH FUCK NO)
void Creature::burn()
{
  if (onfire) {
    hp -= (int)(2*mtrand());
    if (mtrand()>0.99) {
      if (mtrand()>0.9) {
	onfire=0;
      }
    }
  }
}

// Bleeding
void Creature::bleed(Map * map)
{
  if (bleeding >= 0) {

    if ((bleeding % 8 == 0 && bleeding > 40) || bleeding % 16 == 0 ) {
      if (mtrand() > 0.5) {
        hp--;
      }
    }
    bleeding--;
    map->makebloody(y,x);

  }
}

// Healing.
void Creature::heal()
{
  if (hp < maxhp) {
    healcount = healcount-healrate;
    if (healcount <= 0) {
      while (healcount <= 0) {
	if (hp<maxhp) {
	  this->increasestat("toughness");
	  hp++;
	}
	healcount=healcount+1200;
      }
    }
  }
  else {healcount=1200;}

  if (mp < maxmp) {
    rechargecount = rechargecount-rechargerate;
    if (rechargecount <= 0) {
      while (rechargecount <= 0) {
	if (mp<maxmp) {
	  mp++;
	}
	rechargecount=rechargecount+400;
      }
    }
  }
  else {rechargecount=400;}
}

// Suffer poison damage.
void Creature::poison()
{
  if (poisoned > 0) {
    int poisoncalc=poisoned;
//    while (poisoncalc>0) {
      if (poisoncalc > 800*mtrand()+1) {
	hp--;
      }
//      poisoncalc=poisoncalc-400;
//    }
    poisoned--;
  }
}

// Cast a given spell using the monster's mp pool.
int Creature::cast(Spell *spelltocast, Map *map, Screen *vscreen, int targy, int targx)
{
  int cost=spelltocast->level;
  if (cost > mp) {
    vscreen->message("Not enough mp!");
    return 0;
  }
  else {
    if (1 == spelltocast->cast(this, map, vscreen, targy, targx)) {
      time=(char)( (40 / agility) );
      mp=mp-cost;
      return 1;
    }
    else {
      vscreen->message("Nevermind.");
      return 0;
    }
  }
}
