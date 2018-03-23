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

using namespace std;

extern Stuffgetter * objget;

// Player specific functions

// Player constructor
Player::Player(Map *map, int sy, int sx, char ssymb, string sname) : Creature(map, sy, sx, ssymb, sname)
{
  //hp=1000;
    //  maxhp=500;
  //  this->setskill("dodging",2);
  //this->setskill("melee",2);
  //this->setskill("blocking",2);
  isplayer = 1;
  list_add(this->spells(), (void*)objget->spellbyname("frost bolt"));
  maxmp=50;
  mp=50;
  turns=0;
  level=0;
}

// Character creation
void makecharacter(Player * player, Screen *vscreen) {

  int abilities[4];
  int iii,jjj,kkk;
  for (iii=0;iii<4;iii++) {
    abilities[iii]=0;
  }
  while (abilities[0] + abilities[1] + abilities[2] + abilities[3] != 40) {
    for (iii=0;iii<4;iii++) {
      abilities[iii]=0;
      kkk=0;
      for (jjj=0;jjj<3;jjj++) {
	kkk = (int)(6*mtrand() + 1);
	abilities[iii] += kkk;
      }
    }
  }
  
  player->strength=abilities[0];
  player->dexterity=abilities[1];
  player->toughness=abilities[2];
  player->agility=abilities[3];

}

// Player ai definition
// (ai in this case is actual intelligence. Get button presses!)
int Player::ai(Map *map, Screen *vscreen)
{
  string message;
  int ch;
  int doitagain=1;
  int cy, cx;
  string temp;
  if (turns==0) {
    turns++;
    this->calcload();
  }

  if (experience >= XPCURVE * level) {

    int numskills=3;
    if (level==0) {
      numskills=6; this->maxhp=0; this->hp=0;
      //      makecharacter(this, vscreen);
    }
    experience -= XPCURVE * level;
    vscreen->level((Creature*)this, numskills);
    map->plvl = level;
  }

  // Erase set everything as not visible...
  vscreen->hidelevel(map);
  
  // ...then find currently visible squares.
  aoe(map, (float)(this->y+0.5), (float)(this->x), this->viewdist, see);
  aoe(map, (float)(this->y-0.5), (float)(this->x), this->viewdist, see);
  aoe(map, (float)(this->y), (float)(this->x+0.5), this->viewdist, see);
  aoe(map, (float)(this->y), (float)(this->x-0.5), this->viewdist, see);
  
  // Update the screen.
  vscreen->showlevel(map, this->y, this->x);

  if (!frozen) {
    // Heal!
    this->heal();
    
    // Poison D:
    this->poison();
    
    // Bleed D: D: D:
    this->bleed(map);
  }

  // Burn D: D: D: D: D:
  this->burn();

  if (onfire && frozen) {
    if (mtrand() > 0.5) {frozen=0;}
  }

  // Frozen!
  this->freeze();

  if (this->hp < 1) {
    vscreen->message("You die...");
    getch();
    return 0;
  }

  if (time > 0) {time--; return 1;}
  time=(char)( (40 / agility) );
  
  if (load > 0) {
    this->increasestat("strength");
  }

  // All previous messages shown.
  vscreen->nummsg=0;

  Spell * spelltocast;
  objtype kind=spell;
  while (doitagain == 1) {
    
    // Show stats.
    vscreen->stats(this);
    doitagain = 0;
    //    ch = getch();
    ch = vscreen->getachar(map,this->y,this->x);
    Item * item;
    switch(ch)
      {
      case '?': {
	vscreen->help();
	doitagain=1;
	break;
      }
      case 'c':
      case 'o': {
	char action=ch;
	int result;
	int dx,dy;
	if (action=='o') {
	  vscreen->message("> Open in which direction?");
	}
	else {vscreen->message("> Close in which direction?");}
	ch = vscreen->getachar(map,this->y,this->x);
	//	ch = getch();
	switch(ch) {
	case 'y': dx=-1; dy=-1; break;
	case 'u': dx=1; dy=-1; break;
	case 'b': dx=-1; dy=1; break;
	case 'n': dx=1; dy=1; break;
	case 'h':
	case KEY_LEFT: dx=-1; dy=0; break;
	case 'l':
	case KEY_RIGHT: dx=1; dy=0; break;
	case 'k':
	case KEY_UP: dx=0; dy=-1; break;
	case 'j':
	case KEY_DOWN: dx=0; dy=1; break;
	default: dx=-5;dy=-5;break;
	}
	if (dx==-5) {
	  vscreen->message("Nevermind.");
	  dx=0; dy=0;
	  doitagain=1;
	}
	else {
	  if (action == 'o') {
	    result=map->opendoor(y+dy,x+dx);
	  }
	  else {result=map->closedoor(y+dy,x+dx);}
	  if (result==1) {
	    if (action == 'o') {
	      vscreen->message("You open the door.");
	    }
	    else {
	      vscreen->message("You close the door.");
	    }
	    doitagain=0;
	  }
	  else if (result==2) {
	    if (action == 'o') {
	      vscreen->message("That door is already open!");
	    }
	    else {
	      vscreen->message("That door is already closed!");
	    }
	    doitagain=1;
	  }
	  else if (result==3) {
	    vscreen->message("There is something in the way!");
	  }
	  else {
	    vscreen->message("There is no door there!");
	    doitagain=1;
	  }
	}
	break;
      }

      case '>':
	if (map->getfloorsymb(y,x) == '>') {
	  map->changelevel(1, this);
	}
	else {
	  doitagain=1;
	}
	break;
      case '<':
	if (map->getfloorsymb(y,x) == '<') {
	  map->changelevel(-1, this);
	}
	else {
	  doitagain=1;
	}
	break;
      case 'm':
	vscreen->level((Creature*)this, 0); 
	doitagain=1;
	break;
	// Read a scroll.
      case 'r':
	item = vscreen->inventory(this, "Read scroll : ", "scroll");

	if (item != NULL) {
	  //	  vscreen->message("As you read it, the scroll turns to dust.");
	  item->identify();
	  this->read(item, map, vscreen, 0, 0);
	  vscreen->message("As you read it, the scroll turns to dust.");
	  item=NULL;
	  doitagain = 0;
	}
	else {doitagain = 1;}


	break;

	// Zap a wand.
      case 'z':

	item = vscreen->inventory(this, "Zap wand : ", "wand");

	if (item != NULL) {
	  item->identify();
	  this->zap(item, map, vscreen, 0, 0);
	  item=NULL;
	  doitagain = 0;
	}
	else {doitagain = 1;}


	break;

	// Cast spell.
      case 'Z':
	spelltocast=(Spell*)vscreen->choosefromlist(this->spells(),"Choose spell", spell, NULL);
	if (spelltocast != NULL) {
	  doitagain=this->cast(spelltocast,map,vscreen,0,0)+1;
	}
	else {doitagain = 1;}

	break;
	
      case 'Q':
	vscreen->message("Quit without saving? ('Y' or 'N' only)");
	//ch = getch();
	ch = vscreen->getachar(map,this->y,this->x);
	while (ch != 'Y' && ch != 'N') {
	  ch = getch();
	}
	if (ch == 'Y') {
	  vscreen->message("You die...");
	  getch();
	  return 0;
	}
	else {vscreen->message("Okay.");
	  doitagain=1;}
	break;
      case 'S':
	vscreen->message("Save and quit? ('Y' or 'N' only)");
	//ch = getch();
	ch = vscreen->getachar(map,this->y,this->x);
	while (ch != 'Y' && ch != 'N') {
	  //ch = getch();
	  ch = vscreen->getachar(map,this->y,this->x);
	}
	if (ch == 'Y') {return 2;}
	else {vscreen->message("Okay.");
	  doitagain=1;}
	break;

      case 'y':
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,-1,-1,vscreen);
	break;
      case 'u':
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,-1,1,vscreen);
	break;
      case 'b':
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,1,-1,vscreen);
	break;
      case 'n':
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,1,1,vscreen);
	break;
      case 'h':
      case KEY_LEFT:
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,0,-1,vscreen);
	break;
      case 'l':
      case KEY_RIGHT:
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,0,1,vscreen);
	break;
      case 'k':
      case KEY_UP:
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,-1,0,vscreen);
	break;
      case 'j':
      case KEY_DOWN:
        time=(char)( ((40+load/3) / agility) ); // Load slows down motion.
	this->step(map,1,0,vscreen);
	break;

	// Look

      case 'x':
	doitagain=1;
	cy = this->y;
	cx = this->x;
	vscreen->movecursor(map, &cy, &cx);
	break;

	// Examine inventory

      case 'i':
	vscreen->inventory(this, "View Inventory : ", "all");
	doitagain=1;
	break;

	// Get item off floor.

      case 'g':
	if (list_item(map->getitems(y,x)) != NULL) {
	  llist * templist;
	  string itemname;
	  if (list_item(&(*map->getitems(y,x))->next)) {
	    int choosefromlist=1;
	    item = (Item*)list_item(map->getitems(y,x));
	    if (item->stackable == 1) {
	      itemname = item->name;
	      choosefromlist=0;
	      templist = *(map->getitems(y,x));
	      while (templist != NULL) {
		item = (Item*)list_item(&templist);
		if (itemname.compare(item->name) != 0 ) {choosefromlist=1; break;}
		templist = *(&templist->next);
	      }
	    }
	    if (choosefromlist == 1) {
	      temp = "Pick up item";
	      item = (Item*)(vscreen->choosefromlist(map->getitems(y,x), temp, (objtype)1, NULL));
	      if (item == NULL) {doitagain=1; break;}
	    }
	  }
	  else {
	    item = (Item*)list_item(map->getitems(y,x));
	  }
	  if (item->stackable == 0) {
	    vscreen->message("Picked up " + item->getname() + ".");
	    list_add(this->inventory(),item);
	    list_remove(list_search(map->getitems(y,x),(void *)item));
	  }
	  else {
	    //	    llist * templist;
	    /*string */ itemname=item->name;
	    templist = *(map->getitems(y,x));
	    message = "Picked up " + item->getname() + findnumberofitems(&templist, item) + ".";
	    while (templist != NULL && item != NULL) {
	      if (itemname.compare(item->name) != 0) {
		templist = *(&templist->next);
		if (templist == NULL) {break;}
		item = (Item*)list_item(&templist);
		continue;
	      }
	      list_add(this->inventory(),item);
	      list_remove(list_search(map->getitems(y,x),(void *)item));
	      templist = *(map->getitems(y,x));
	      if (templist == NULL) {break;}
	      item = (Item *)list_item(&templist);
	    }
	    vscreen->message(message);//"Picked up multiple " + item->getname() + ".");
	  }
	  this->calcload();
	}
	else {
	  vscreen->message("There is nothing here!");
	  doitagain = 1;
	}
	break;

	// Wear clothing/armor.

      case 'W':
	item = vscreen->inventory(this, "Wear Item : ", "armor");
	if (item != NULL) {
	  string subcat = this->wearingarmor(item->getsubcat());
	  if (item == lhand || item == rhand) {
	    vscreen->message("Stop wielding it first!");
	    doitagain=1;
	  }
	  else {
	    if (list_search(this->armor(),(void *)item) != NULL) {
	      list_remove(list_search(this->armor(),(void *)item));
	      vscreen->message("Took off " + item->getname() + ".");
	      doitagain=1;
	    }
	    else {
	      if (subcat.compare("none") == 0) {
		list_add(this->armor(),(void *)item);
		vscreen->message("Put on " + item->getname() + ".");
	      }
	      else {doitagain=1; vscreen->message("Take off your " + subcat + " first!");}
	    }
	  }
	}
	else {doitagain = 1; vscreen->message("Nevermind.");}
	break;

	// Launch a missile
      case 'f':
	item=NULL;
	if (lhand != NULL) {
	  if ((lhand->gettype()).compare("launcher")==0) {item=lhand;}
	}
	if (rhand != NULL) {
	  if ((rhand->gettype()).compare("launcher")==0) {item=rhand;}
	}
	if (item == NULL) {
	  doitagain=1;
	  vscreen->message("Not wielding a launcher!");
	}
	else {
	  Item * missile;
	  missile = this->getmissile(item);
	  if (missile == NULL) {
	    doitagain=1;
	    vscreen->message("No appropriate missile!");
	  }
	  else {
	    cy = this->y;
	    cx = this->x;
	    vscreen->movecursor(map, &cy, &cx);
	    if (cy != y || cx != x ) {
	      this->fire(cy,cx,map,vscreen,16,missile, item);
	      
	      this->unwear(missile);
	      this->unwield(missile);
	      if (list_search(this->armor(),(void *)missile) != NULL) {
		list_remove(list_search(this->armor(),(void *)missile));
	      }
	    }
	    else {doitagain = 1;}
	    
	  }
	}
	break;

	// Throw stuff.

      case 't':
	item = vscreen->inventory(this, "Throw Item : ", "all");
	if (item != NULL) {
	  cy = this->y;
	  cx = this->x;
	  vscreen->movecursor(map, &cy, &cx);
	  if (cy != y || cx != x ) {
	    this->fire(cy,cx,map,vscreen,8,item);

	    this->unwear(item);
	    this->unwield(item);
	    if (list_search(this->armor(),(void *)item) != NULL) {
	      list_remove(list_search(this->armor(),(void *)item));
	    }
	  }
	  else {doitagain = 1;}
	  
	}
	else {doitagain = 1;}
	break;
	
	// Quaff potion
	
      case 'q':
	item = vscreen->inventory(this, "Quaff potion : ", "potion");
	if (item != NULL) {
	  this->quaff(item,vscreen);
	  delete item;
	  item=NULL;
	}
	else {doitagain = 1; vscreen->message("Nevermind.");}
	break;

	// Drop item onto floor.

      case 'd':
	item = vscreen->inventory(this, "Drop Inventory Item : ", "all");
	if (item != NULL) {
	  llist * templist;
	  templist = possessions;
	  string itemname=item->name;
	  message = "Dropped " + item->getname() + findnumberofitems(&templist, item) + ".";
	  while (item != NULL) {
	    if (itemname.compare(item->name) != 0) {

	      templist = *(&templist->next);
	      if (templist == NULL) {break;}
	      item = (Item *)list_item(&templist);
	      continue;

	    }
	    list_add(map->getitems(y,x),(void *)item);
	    list_remove(list_search(this->inventory(),(void *)item));
	    templist = possessions;
	    item->y = y;
	    item->x = x;
	    if (this->unwear(item)==1) {}//message = "Took off " + item->getname() + " and dropped it.";}
	    else if (this->unwield(item)==1) {}//message = "Dropped " + item->getname() + ".";}
	    else if (list_search(this->armor(),(void *)item) != NULL) {
	      list_remove(list_search(this->armor(),(void *)item));
	      //	      message = "Took off " + item->getname() + " and dropped it.";
	    }
	    else {}//message = "Dropped " + item->getname() + ".";}
	    if (item->stackable == 0) {break;}
	    else {
	      //	      templist = *(&templist->next);
	      if (templist == NULL) {break;}
	      item = (Item *)list_item(&templist);
	    }
	  }
	  this->calcload();
	  vscreen->message(message);
	}
	else {doitagain = 1;
	  vscreen->message("Nevermind.");
	}
	break;

	// Wield item in hand.

      case 'w':
	item = vscreen->inventory(this, "Wield/Unwield Item : ", "all");
	disarmed=0;
	sundered=0;
	if (item != NULL) {
	  int itworked;
	  itworked = this->wield(item);
	  if (itworked == 0) {
	    vscreen->message("Unwield " + lhand->getname() + " [l] or " + rhand->getname() + " [r]?");
	    int chg;
	    //chg = getch();
	    chg = vscreen->getachar(map,this->y,this->x);
	    if (chg == 'l') {
	      vscreen->message("Unwielded " + lhand->getname() + " and wielded " + item->getname() + ".");
	      this->unwield(lhand);this->wield(item);
	    }
	    else if (chg == 'r') {
	      vscreen->message("Unwielded " + rhand->getname() + " and wielded " + item->getname() + ".");
	      this->unwield(rhand);this->wield(item);
	    }
	    else {doitagain=1; vscreen->message("Nevermind.");}
	  }
	  else if (itworked == 2) {this->unwield(item);
	    vscreen->message("Unwielded " + item->getname() + ".");
	    doitagain=1;
	  }
	  else if (itworked == 3) {
	    vscreen->message("Take it off, first!");
	    doitagain=1;
	  }
	  else if (itworked == 4) {
            vscreen->message("Unwielded " + rhand->getname() + " and wielded " + item->getname() + ".");
            this->unwield(rhand);this->wield(item);
	  }
          else if (itworked == 5) {
            vscreen->message("Unwielded " + lhand->getname() + " and wielded " + item->getname() + ".");
            this->unwield(lhand);this->wield(item);
          }
          else if (itworked == 6) {
            vscreen->message("Unwielded " + rhand->getname() + " and " + lhand->getname() + " and wielded " + item->getname() + ".");
            this->unwield(rhand);this->unwield(lhand);this->wield(item);
          }
	  else {vscreen->message("Wielding " + item->getname() + ".");}
	}
	break;

	// Put on jewellry.

      case 'P':
	item = vscreen->inventory(this, "Put on/Take off jewellry : ", "ring,amulet");
	if (item != NULL) {
	  int itworked;
	  itworked = this->wear(item);
	  if (itworked == 0) {
	    vscreen->message("Take off " + lring->getname() + " [l] or " + rring->getname() + " [r]?");
	    int chg;
	    //chg = getch();
	    chg = vscreen->getachar(map,this->y,this->x);
	    if (chg == 'l') {
	      vscreen->message("Took off " + lring->getname() + " and put on " + item->getname() + ".");
	      this->unwear(lring);this->wear(item);
	    }
	    else if (chg == 'r') {
	      vscreen->message("Took off " + rring->getname() + " and put on " + item->getname() + ".");
	      this->unwear(rring);this->wear(item);
	    }
	    else {doitagain=1; vscreen->message("Nevermind.");}
	  }
	  else if (itworked == 2) {this->unwear(item);
	    vscreen->message("Took off " + item->getname() + ".");
	    doitagain=1;
	  }
	  else {vscreen->message("Put on " + item->getname() + ".");}
	}
	else {doitagain = 1;}
	break;
      default:
	doitagain = 1;
      }
  }

  // Return 1 --> Still playing! Not dead. Not victorious. Not anything!
  return 1;

}
